#include "database.h"
#include <QSqlError>
#include <QDebug>

bool Database::initialize() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("finance_tracker.db");  // The database file

    if (!db.open()) {
        qDebug() << "Database error:" << db.lastError().text();
        return false;
    }

    // Create transactions table if it doesn't exist
    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            category TEXT NOT NULL,
            description TEXT,
            amount REAL NOT NULL
        )
    )";

    if (!query.exec(createTable)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::addTransaction(const QString &date, const QString &category, const QString &description, double amount) {
    qDebug() << "Inserting: " << date << category << description << amount;
    
    QSqlQuery query;
    query.prepare("INSERT INTO transactions (date, category, description, amount) VALUES (?, ?, ?, ?)");
    query.addBindValue(date);
    query.addBindValue(category);
    query.addBindValue(description);
    query.addBindValue(amount);

    if (!query.exec()) {
        qDebug() << "Failed to add transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::updateTransaction(int id, const QString &date, const QString &category, const QString &description, double amount) {
    QSqlQuery query;
    query.prepare("UPDATE transactions SET date = ?, category = ?, description = ?, amount = ? WHERE id = ?");
    query.addBindValue(date);
    query.addBindValue(category);
    query.addBindValue(description);
    query.addBindValue(amount);
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
    QSqlQuery query("SELECT id, date, category, description, amount FROM transactions ORDER BY date DESC");
    return query;
}