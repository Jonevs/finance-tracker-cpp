#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

class Database {
public:
    static bool initialize();
    static bool addTransaction(const QString &date, const QString &category, const QString &description, double amount);
    static QSqlQuery getAllTransactions();
    static bool updateTransaction(int id, const QString &date, const QString &category, const QString &description, double amount);  
    static bool deleteTransaction(int id);  
};

#endif // DATABASE_H
