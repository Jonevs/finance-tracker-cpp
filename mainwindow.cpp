#include "mainwindow.h"
#include "database.h"
#include <QLabel>
#include <QHeaderView>  
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug> 

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUI();

    if (!Database::initialize()) {
        QMessageBox::critical(this, "Database Error", "Failed to connect to the database.");
    } else {
        loadTransactions();  
    }
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *formLayout = new QHBoxLayout();

    dateInput = new QDateEdit(QDate::currentDate(), this);
    dateInput->setCalendarPopup(true);
    formLayout->addWidget(new QLabel("Date:"));
    formLayout->addWidget(dateInput);

    categoryInput = new QComboBox(this);
    categoryInput->addItems({"Food", "Rent", "Entertainment", "Transport", "Other"});
    formLayout->addWidget(new QLabel("Category:"));
    formLayout->addWidget(categoryInput);

    descriptionInput = new QLineEdit(this);
    formLayout->addWidget(new QLabel("Description:"));
    formLayout->addWidget(descriptionInput);

    amountInput = new QLineEdit(this);
    amountInput->setPlaceholderText("0.00");
    formLayout->addWidget(new QLabel("Amount:"));
    formLayout->addWidget(amountInput);

    addButton = new QPushButton("Add Transaction", this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTransaction);
    formLayout->addWidget(addButton);

    editButton = new QPushButton("Edit Transaction", this);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editTransaction);
    editButton->setEnabled(false);  
    formLayout->addWidget(editButton);

    deleteButton = new QPushButton("Delete Transaction", this);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTransaction);
    deleteButton->setEnabled(false);  
    formLayout->addWidget(deleteButton);

    mainLayout->addLayout(formLayout);

    transactionTable = new QTableWidget(this);
    transactionTable->setColumnCount(5);
    transactionTable->setHorizontalHeaderLabels({"ID", "Date", "Category", "Description", "Amount"});
    transactionTable->setColumnHidden(0, true);
    transactionTable->horizontalHeader()->setStretchLastSection(true);
    transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  
    transactionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(transactionTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTransactionSelected);

    mainLayout->addWidget(transactionTable);

    setCentralWidget(centralWidget);
    setWindowTitle("Personal Finance Tracker");
    resize(800, 400);
}

void MainWindow::clearForm() {
    descriptionInput->clear();
    amountInput->clear();
    dateInput->setDate(QDate::currentDate());
    categoryInput->setCurrentIndex(0);
    selectedTransactionId = -1;
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

void MainWindow::addTransaction() {
    QString date = dateInput->date().toString("yyyy-MM-dd");
    QString category = categoryInput->currentText();
    QString description = descriptionInput->text();
    QString amountText = amountInput->text();

    if (description.isEmpty() || amountText.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    bool ok;
    double amount = amountText.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid amount.");
        return;
    }

    if (Database::addTransaction(date, category, description, amount)) {
        loadTransactions();

        descriptionInput->clear();
        amountInput->clear();
        dateInput->setDate(QDate::currentDate());
        categoryInput->setCurrentIndex(0);
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to add transaction.");
    }
}

void MainWindow::loadTransactions() {
    transactionTable->setRowCount(0);  

    QSqlQuery query = Database::getAllTransactions();
    int row = 0;

    while (query.next()) {
        qDebug() << "Loading: " << query.value(0).toString() << query.value(1).toString() << query.value(2).toString() << query.value(3).toString() << query.value(4).toDouble();
        transactionTable->insertRow(row);
        transactionTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));  // ID (hidden)
        transactionTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));  // Date
        transactionTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));  // Categoryd
        transactionTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));  // Description (fixed)
        transactionTable->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(4).toDouble(), 'f', 2)));  // Amount
        row++;
    }
}

void MainWindow::onTransactionSelected() {
    int currentRow = transactionTable->currentRow();  

    if (currentRow == -1) {  
        qDebug() << "No row selected.";
        clearForm();
        editButton->setEnabled(false);
        deleteButton->setEnabled(false);
        return;
    }

    // Retrieve data from the selected row
    selectedTransactionId = transactionTable->item(currentRow, 0)->text().toInt();
    qDebug() << "Selected Transaction ID:" << selectedTransactionId;

    // Populate the form fields with the selected row's data
    dateInput->setDate(QDate::fromString(transactionTable->item(currentRow, 1)->text(), "yyyy-MM-dd"));
    categoryInput->setCurrentText(transactionTable->item(currentRow, 2)->text());
    descriptionInput->setText(transactionTable->item(currentRow, 3)->text());
    amountInput->setText(transactionTable->item(currentRow, 4)->text());

    // Enable Edit and Delete buttons after selection
    editButton->setEnabled(true);
    deleteButton->setEnabled(true);
}

void MainWindow::editTransaction() {
    if (selectedTransactionId == -1) return;

    QString date = dateInput->date().toString("yyyy-MM-dd");
    QString category = categoryInput->currentText();
    QString description = descriptionInput->text();
    QString amountText = amountInput->text();

    if (description.isEmpty() || amountText.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    bool ok;
    double amount = amountText.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid amount.");
        return;
    }

    // Update the transaction in the database
    if (Database::updateTransaction(selectedTransactionId, date, category, description, amount)) {
        QMessageBox::information(this, "Success", "Transaction updated successfully.");
        loadTransactions();  
        clearForm();      
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update transaction.");
    }
}

void MainWindow::deleteTransaction() {
    if (selectedTransactionId == -1) return;

    // Confirm deletion
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Transaction", "Are you sure you want to delete this transaction?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (Database::deleteTransaction(selectedTransactionId)) {
            loadTransactions();  // Refresh the table
            clearForm();         // Clear form after deletion
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to delete transaction.");
        }
    }
}

