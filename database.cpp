#include "database.h"
#include <QSqlError>
#include <QDebug>

bool Database::initialize() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("finance_tracker.db"); 

    if (!db.open()) {
        qDebug() << "Database error:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            category TEXT NOT NULL,
            description TEXT,
            amount REAL NOT NULL,
            type TEXT NOT NULL DEFAULT 'expense' 
        )
    )";

    if (!query.exec(createTable)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::addTransaction(const QString &date, const QString &category, const QString &description, double amount, const QString &type) {
    qDebug() << "Inserting: " << date << category << description << amount;
    
    QSqlQuery query;
    query.prepare("INSERT INTO transactions (date, category, description, amount, type) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(date);
    query.addBindValue(category);
    query.addBindValue(description);
    query.addBindValue(amount);
    query.addBindValue(type);

    if (!query.exec()) {
        qDebug() << "Failed to add transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::updateTransaction(int id, const QString &date, const QString &category, const QString &description, double amount, const QString &type) {
    QSqlQuery query;
    query.prepare("UPDATE transactions SET date = ?, category = ?, description = ?, amount = ?, type = ? WHERE id = ?");
    query.addBindValue(date);
    query.addBindValue(category);
    query.addBindValue(description);
    query.addBindValue(amount);
    query.addBindValue(type);
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Failed to update transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::deleteTransaction(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM transactions WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Failed to delete transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

QSqlQuery Database::getAllTransactions() {
    QSqlQuery query("SELECT id, date, category, description, amount, type FROM transactions ORDER BY date DESC");
    return query;
}