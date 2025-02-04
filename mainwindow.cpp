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
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "customtablewidget.h"

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
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10); 

    // ================= FORM SECTION =================
    QHBoxLayout *formLayout = new QHBoxLayout();
    formLayout->setSpacing(10);

    // Form Inputs
    formLayout->addWidget(new QLabel("Date:"));
    dateInput = createDateEdit(QDate::currentDate());
    formLayout->addWidget(dateInput);

    formLayout->addWidget(new QLabel("Type:"));
    typeInput = createComboBox({"Expense", "Income"});
    formLayout->addWidget(typeInput);

    formLayout->addWidget(new QLabel("Category:"));
    categoryInput = createComboBox({"Food", "Rent", "Entertainment", "Transport", "Other"});
    formLayout->addWidget(categoryInput);

    formLayout->addWidget(new QLabel("Description:"));
    descriptionInput = new QLineEdit(this);
    formLayout->addWidget(descriptionInput);

    formLayout->addWidget(new QLabel("Amount:"));
    amountInput = new QLineEdit(this);
    amountInput->setPlaceholderText("0.00");
    formLayout->addWidget(amountInput);

    // Form Buttons
    addButton = createStyledButton("Add", "#4CAF50", "#45A049");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTransaction);
    formLayout->addWidget(addButton);

    editButton = createStyledButton("Edit", "#2196F3", "#1976D2", true);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editTransaction);
    formLayout->addWidget(editButton);

    deleteButton = createStyledButton("Delete", "#F44336", "#D32F2F", true);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteTransaction);
    formLayout->addWidget(deleteButton);

    mainLayout->addLayout(formLayout);

    // ================= FILTER TOGGLE BUTTON =================
    QPushButton *showFiltersButton = createStyledButton("Show Filters", "#607D8B", "#455A64");
    mainLayout->addWidget(showFiltersButton);

    // ================= FILTER SECTION =================
    QWidget *filterContainer = new QWidget(this);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterContainer);
    filterLayout->setContentsMargins(0, 0, 0, 0);  
    filterContainer->setVisible(false);
    mainLayout->addWidget(filterContainer);

    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Search Description...");
    filterLayout->addWidget(searchInput);

    filterCategory = createComboBox({"All Categories", "Food", "Rent", "Entertainment", "Transport", "Other"});
    filterLayout->addWidget(filterCategory);

    filterType = createComboBox({"All Types", "Income", "Expense"});
    filterLayout->addWidget(filterType);

    filterLayout->addWidget(new QLabel("From:"));
    filterStartDate = createDateEdit(QDate::currentDate().addMonths(-1));
    filterLayout->addWidget(filterStartDate);

    filterLayout->addWidget(new QLabel("To:"));
    filterEndDate = createDateEdit(QDate::currentDate());
    filterLayout->addWidget(filterEndDate);

    QPushButton *applyFiltersButton = createStyledButton("Apply Filters", "#2196F3", "#1976D2");
    connect(applyFiltersButton, &QPushButton::clicked, this, &MainWindow::applyFilters);
    filterLayout->addWidget(applyFiltersButton);

    QPushButton *clearFiltersButton = createStyledButton("Clear Filters", "#9E9E9E", "#757575");
    connect(clearFiltersButton, &QPushButton::clicked, this, &MainWindow::clearFilters);
    filterLayout->addWidget(clearFiltersButton);

    connect(showFiltersButton, &QPushButton::clicked, this, [=]() {
        bool isVisible = filterContainer->isVisible();
        filterContainer->setVisible(!isVisible);  
        showFiltersButton->setText(isVisible ? "Show Filters" : "Hide Filters");
    });

    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::applyFilters);

    // ================= TRANSACTION TABLE =================
    transactionTable = new CustomTableWidget(this);
    transactionTable->setStyleSheet("QTableWidget::item:selected { background-color:rgb(115, 139, 160); }");
    transactionTable->setColumnCount(6);
    transactionTable->setHorizontalHeaderLabels({"ID", "Date", "Category", "Description", "Amount", "Type"});
    transactionTable->setColumnHidden(0, true);
    transactionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    transactionTable->resizeRowsToContents();
    transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  
    transactionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(transactionTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTransactionSelected);
    connect(transactionTable, &CustomTableWidget::rowDeselected, this, &MainWindow::clearForm);
    mainLayout->addWidget(transactionTable);

    // ================= EXPORT CSV BTN =================
    QPushButton *exportButton = createStyledButton("Export to CSV", "#FF9800", "#FB8C00");
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);
    mainLayout->addWidget(exportButton);

    setCentralWidget(centralWidget);
    setWindowTitle("Personal Finance Tracker by Jonevs");
    resize(800, 400);
}

QPushButton* MainWindow::createStyledButton(const QString &text, const QString &color, const QString &hoverColor, bool disabled) {
    QPushButton *button = new QPushButton(text, this);
    QString style = QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            font-weight: bold;
            border-radius: 5px;
            padding: 5px 10px;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:disabled {
            background-color: #D3D3D3;
            color: #A9A9A9;
            font-weight: normal;
            border: 1px solid #A9A9A9;
        }
    )").arg(color, hoverColor);
    button->setStyleSheet(style);
    button->setEnabled(!disabled);
    return button;
}

QComboBox* MainWindow::createComboBox(const QStringList &items) {
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(items);
    return comboBox;
}

QDateEdit* MainWindow::createDateEdit(const QDate &date) {
    QDateEdit *dateEdit = new QDateEdit(date, this);
    dateEdit->setCalendarPopup(true);
    return dateEdit;
}

void MainWindow::clearForm() {
    descriptionInput->clear();
    amountInput->clear();
    dateInput->setDate(QDate::currentDate());
    categoryInput->setCurrentIndex(0);
    typeInput->setCurrentIndex(0);
    selectedTransactionId = -1;
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

void MainWindow::clearFilters() {
    // Reset all filter inputs to their default values
    searchInput->clear();
    filterCategory->setCurrentIndex(0);  // "All Categories"
    filterType->setCurrentIndex(0);      // "All Types"
    filterStartDate->setDate(QDate::currentDate().addMonths(-1));  // Last month
    filterEndDate->setDate(QDate::currentDate());                 // Today

    // Reload all transactions without filters
    loadTransactions();
}

void MainWindow::addTransaction() {
    QString date = dateInput->date().toString("yyyy-MM-dd");
    QString category = categoryInput->currentText();
    QString description = descriptionInput->text();
    QString amountText = amountInput->text();
    QString type = typeInput->currentText();

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

    if (Database::addTransaction(date, category, description, amount, type)) {
        loadTransactions();
        clearForm(); 
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to add transaction.");
    }
}

void MainWindow::loadTransactions() {
    transactionTable->setRowCount(0);  

    QSqlQuery query = Database::getAllTransactions();
    int row = 0;

    while (query.next()) {
        transactionTable->insertRow(row);
        transactionTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));  // ID (hidden)
        transactionTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));  // Date
        transactionTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));  // Categoryd
        transactionTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));  // Description (fixed)
        transactionTable->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(4).toDouble(), 'f', 2)));  // Amount
        QString type = query.value(5).toString();  // Type (income/expense)
        QTableWidgetItem *typeItem = new QTableWidgetItem(type);

        if (type == "Income") {
            typeItem->setForeground(QColor("green"));
        } else {
            typeItem->setForeground(QColor("red"));
        }

        transactionTable->setItem(row, 5, typeItem); 
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
    typeInput->setCurrentText(transactionTable->item(currentRow, 5)->text());

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
    QString type = typeInput->currentText();

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
    if (Database::updateTransaction(selectedTransactionId, date, category, description, amount, type)) {
        QMessageBox::information(this, "Success", "Transaction updated successfully.");
        loadTransactions();  
        clearForm();      
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update transaction.");
    }
}

void MainWindow::deleteTransaction() {
    if (selectedTransactionId == -1) return;

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

void MainWindow::applyFilters() {
    QString searchText = searchInput->text().trimmed();
    QString selectedCategory = filterCategory->currentText();
    QString selectedType = filterType->currentText();
    QString startDate = filterStartDate->date().toString("yyyy-MM-dd");
    QString endDate = filterEndDate->date().toString("yyyy-MM-dd");

    transactionTable->setRowCount(0);  

    QSqlQuery query;
    QString sqlQuery = "SELECT * FROM transactions WHERE date BETWEEN ? AND ?";
    QList<QVariant> bindValues = {startDate, endDate};

    // Add filters dynamically
    if (selectedCategory != "All Categories") {
        sqlQuery += " AND category = ?";
        bindValues.append(selectedCategory);
    }
    if (selectedType != "All Types") {
        sqlQuery += " AND type = ?";
        bindValues.append(selectedType);  
    }
    if (!searchText.isEmpty()) {
        sqlQuery += " AND description LIKE ?";
        bindValues.append("%" + searchText + "%");
    }

    query.prepare(sqlQuery);
    for (const auto &value : bindValues) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        qDebug() << "Filter query failed:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        transactionTable->insertRow(row);
        transactionTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));  // ID
        transactionTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));  // Date
        transactionTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));  // Category
        transactionTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));  // Description
        transactionTable->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(4).toDouble(), 'f', 2)));  // Amount

        QString type = query.value(5).toString();
        QTableWidgetItem *typeItem = new QTableWidgetItem(type);
        if (type == "Income") {
            typeItem->setForeground(QColor("green"));
        } else {
            typeItem->setForeground(QColor("red"));
        }
        transactionTable->setItem(row, 5, typeItem);
        row++;
    }
}

void MainWindow::exportToCSV() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Transactions", "", "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return; 
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    out << "Date,Category,Description,Amount,Type\n";

    for (int row = 0; row < transactionTable->rowCount(); ++row) {
        QString date = transactionTable->item(row, 1)->text();
        QString category = transactionTable->item(row, 2)->text();
        QString description = transactionTable->item(row, 3)->text().replace(",", " "); 
        QString amount = transactionTable->item(row, 4)->text();
        QString type = transactionTable->item(row, 5)->text();

        out << QString("%1,%2,%3,%4,%5\n").arg(date, category, description, amount, type);
    }

    file.close();

    QMessageBox::information(this, "Export Successful", "Transactions have been exported successfully.");
}