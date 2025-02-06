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
    mainLayout->setSpacing(20);

    // ================= FILTER TOGGLE & EXPORT BUTTONS =================
    QHBoxLayout *topButtonLayout = new QHBoxLayout(); 
    topButtonLayout->setAlignment(Qt::AlignLeft);     
    topButtonLayout->setSpacing(10); 

    QPushButton *showFiltersButton = createStyledButton("Show Filters", "#607D8B", "#455A64");
    showFiltersButton->setFixedWidth(120);  
    topButtonLayout->addWidget(showFiltersButton);

    QPushButton *exportButton = createStyledButton("Export CSV", "#FF9800", "#FB8C00");
    exportButton->setFixedWidth(120);  
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToCSV);
    topButtonLayout->addWidget(exportButton);

    QPushButton *toggleDarkModeButton = createStyledButton("Dark Mode", "#9E9E9E", "#757575");
    toggleDarkModeButton->setFixedWidth(120);
    connect(toggleDarkModeButton, &QPushButton::clicked, this, [=]() {
        darkModeEnabled = !darkModeEnabled;

        if (darkModeEnabled) {
            this->setStyleSheet(getDarkModeStyle());
            toggleDarkModeButton->setText("Light Mode");
        } else {
            this->setStyleSheet("");  
            toggleDarkModeButton->setText("Dark Mode");
        }
        updateTableColors();
    });
    topButtonLayout->addWidget(toggleDarkModeButton);


    mainLayout->addLayout(topButtonLayout);

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
    transactionTable->setHorizontalHeaderLabels({"ID", "Date (Y-m-d)", "Category", "Description", "Amount", "Type"});
    transactionTable->setColumnHidden(0, true);
    transactionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    transactionTable->resizeRowsToContents();
    transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  
    transactionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionTable->setSortingEnabled(true);

    transactionTable->horizontalHeader()->setStyleSheet(R"(
        QHeaderView::section {
            background-color: #607D8B;
            color: white;
            font-weight: bold;
            padding: 5px;
            border: 1px solid #455A64;
        }

        QHeaderView::up-arrow, QHeaderView::down-arrow {
            width: 0px;  
            height: 0px;
        }
    )");

    transactionTable->horizontalHeader()->setToolTip("Shortcuts: Ctrl+D to sort by Date, Ctrl+A to sort by Amount");
    transactionTable->horizontalHeader()->setToolTip("Single-click to sort, double-click to clear sorting");
    this->setFocus();

    connect(transactionTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::sortTable);
    connect(transactionTable->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this, &MainWindow::clearSorting);
    connect(transactionTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTransactionSelected);
    connect(transactionTable, &CustomTableWidget::rowDeselected, this, &MainWindow::clearForm);
    mainLayout->addWidget(transactionTable);

    // Ctrl + D for Date Sorting
    sortByDateShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(sortByDateShortcut, &QShortcut::activated, this, [=]() {
        transactionTable->sortItems(1, dateSortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
        updateHeaderArrows(1, dateSortAscending);  // Update header arrows for Date
        dateSortAscending = !dateSortAscending;
    });

    // Ctrl + A for Amount Sorting
    sortByAmountShortcut = new QShortcut(QKeySequence("Ctrl+A"), this);
    connect(sortByAmountShortcut, &QShortcut::activated, this, [=]() {
        transactionTable->sortItems(4, amountSortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
        updateHeaderArrows(4, amountSortAscending);  // Update header arrows for Amount
        amountSortAscending = !amountSortAscending;
    });

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

void MainWindow::clearSorting() {
    if (currentSortedColumn == -1) return;  

    transactionTable->setSortingEnabled(false);  
    loadTransactions();  
    transactionTable->setSortingEnabled(true);   

    updateHeaderArrows(-1, true); 
    currentSortedColumn = -1;  
    highlightSortedColumn();   
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
        transactionTable->setSortingEnabled(false);
        loadTransactions();
        transactionTable->setSortingEnabled(true); 
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
        qDebug() << "Loading row:" << row << query.value(0).toString();
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
    qDebug() << "Total rows loaded:" << row;
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

    if (!transactionTable->item(currentRow, 0)) return;  // Prevent null reference crash
    selectedTransactionId = transactionTable->item(currentRow, 0)->text().toInt();

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
            loadTransactions();  
            clearForm();         
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to delete transaction.");
        }
    }
}

void MainWindow::applyFilters() {
    QString searchTerm = searchInput->text().trimmed().toLower();
    QString selectedCategory = filterCategory->currentText();
    QString selectedType = filterType->currentText();
    QDate startDate = filterStartDate->date();
    QDate endDate = filterEndDate->date();

    // Iterate over all rows and apply filters
    for (int row = 0; row < transactionTable->rowCount(); ++row) {
        bool showRow = true;

        // Defensive Checks: Ensure all table items exist
        QTableWidgetItem *dateItem = transactionTable->item(row, 1);
        QTableWidgetItem *categoryItem = transactionTable->item(row, 2);
        QTableWidgetItem *descriptionItem = transactionTable->item(row, 3);
        QTableWidgetItem *amountItem = transactionTable->item(row, 4);
        QTableWidgetItem *typeItem = transactionTable->item(row, 5);

        // If any critical data is missing, hide the row
        if (!dateItem || !categoryItem || !descriptionItem || !amountItem || !typeItem) {
            transactionTable->setRowHidden(row, true);
            continue;  // Skip to the next row
        }

        // Filter by Date Range
        QDate transactionDate = QDate::fromString(dateItem->text(), "yyyy-MM-dd");
        if (!transactionDate.isValid() || transactionDate < startDate || transactionDate > endDate) {
            showRow = false;
        }

        // Filter by Search Term in Description
        if (!descriptionItem->text().toLower().contains(searchTerm)) {
            showRow = false;
        }

        // Filter by Category
        if (selectedCategory != "All Categories" && categoryItem->text() != selectedCategory) {
            showRow = false;
        }

        // Filter by Type (Income/Expense)
        if (selectedType != "All Types" && typeItem->text() != selectedType) {
            showRow = false;
        }

        // Filter by Valid Amount (Handle invalid numbers safely)
        bool ok;
        double amount = amountItem->text().toDouble(&ok);
        if (!ok) {
            showRow = false;  // Hide rows with invalid amounts
        }

        // Apply final visibility
        transactionTable->setRowHidden(row, !showRow);
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

void MainWindow::sortTable(int column) {
    bool ascending = columnSortOrder.value(column, true);
    transactionTable->sortItems(column, ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
    updateHeaderArrows(column, ascending);  
    columnSortOrder[column] = !ascending; 
}

void MainWindow::updateHeaderArrows(int sortedColumn, bool ascending) {
    currentSortedColumn = sortedColumn;

    if (darkModeEnabled) {
        this->setStyleSheet(getDarkModeStyle());  
    }

    for (int i = 0; i < transactionTable->columnCount(); ++i) {
        QString headerText = transactionTable->horizontalHeaderItem(i)->text();
        headerText = headerText.remove(" ▲").remove(" ▼");  // Remove existing arrows

        if (i == sortedColumn) {
            headerText += ascending ? " ▲" : " ▼";  // Add arrow for the sorted column

            // Apply dark mode styling for the sorted column
            transactionTable->horizontalHeaderItem(i)->setBackground(QColor("#546E7A"));  // Darker background in dark mode
            transactionTable->horizontalHeaderItem(i)->setForeground(QBrush(Qt::white));  // White text for contrast
        } else {
            // Reset styles for non-sorted columns
            transactionTable->horizontalHeaderItem(i)->setBackground(QBrush(Qt::transparent));
            transactionTable->horizontalHeaderItem(i)->setForeground(QBrush(Qt::white));
        }

        transactionTable->horizontalHeaderItem(i)->setText(headerText);  // Update header text
    }

    highlightSortedColumn();  // Ensure the column highlight adapts to dark mode
}

void MainWindow::highlightSortedColumn() {
    for (int row = 0; row < transactionTable->rowCount(); ++row) {
        for (int col = 0; col < transactionTable->columnCount(); ++col) {
            QTableWidgetItem *item = transactionTable->item(row, col);

            if (item) {
                // Highlight the sorted column
                if (col == currentSortedColumn) {
                    if (darkModeEnabled) {
                        item->setBackground(QColor("#546E7A"));  // Dark mode highlight
                    } else {
                        item->setBackground(QColor("#FFF9C4"));  // Light mode highlight
                    }
                } else {
                    item->setBackground(QBrush(Qt::transparent));  // Reset background for non-sorted columns
                }

                // Preserve Income/Expense colors in the "Type" column (column 5)
                if (col == 5) {  
                    QString type = item->text();
                    if (type == "Income") {
                        item->setForeground(QColor("#00C853"));  
                    } else if (type == "Expense") {
                        item->setForeground(QColor("#FF5252")); 
                    }
                } else {
                    // Apply general text color for other columns based on the mode
                    item->setForeground(QBrush(darkModeEnabled ? Qt::white : Qt::black));
                }
            }
        }
    }
}

QString MainWindow::getDarkModeStyle() {
    return R"(
        QWidget {
            background-color: #2E2E2E;
            color: #FFFFFF;
        }

        QLineEdit, QComboBox, QDateEdit {
            background-color: #424242;
            border: 1px solid #616161;
            color: #FFFFFF;
            padding: 5px;
            border-radius: 4px;
        }

        /* Transaction Table Styles */
        QTableWidget {
            background-color: #424242;
            color: #FFFFFF;
            gridline-color: #616161;
            selection-background-color: #546E7A;  /* Selected Row Background */
            selection-color: #FFFFFF;             /* Selected Text Color */
        }

        /* Table Header Styles */
        QHeaderView::section {
            background-color: #37474F;
            color: #FFFFFF;
            font-weight: bold;
            padding: 5px;
            border: 1px solid #455A64;
        }

        /* Hover Effect for Table Rows */
        QTableWidget::item:hover {
            background-color: #546E7A;  /* Hover background color */
            color: #FFFFFF;             /* Hover text color */
        }

        QHeaderView::section.sorted {
            background-color: #546E7A;  /* Darker background for sorted column */
            color: #FFFFFF;
            font-weight: bold;
        }

        QPushButton {
            background-color: #616161;
            color: #FFFFFF;
            border-radius: 4px;
            padding: 5px 10px;
        }

        QPushButton:hover {
            background-color: #757575;
        }

        QPushButton:disabled {
            background-color: #9E9E9E;
            color: #BDBDBD;
        }

        QScrollBar:vertical {
            background: #333333;
            width: 10px;
        }

        QScrollBar::handle:vertical {
            background: #757575;
            min-height: 20px;
        }

        QLabel {
            color: #FFFFFF;
        }
    )";
}

void MainWindow::updateTableColors() {
    for (int row = 0; row < transactionTable->rowCount(); ++row) {
        for (int col = 0; col < transactionTable->columnCount(); ++col) {
            QTableWidgetItem *item = transactionTable->item(row, col);

            if (item) {
                // Check if this is the sorted column
                if (col == currentSortedColumn) {
                    // Apply highlight based on the mode
                    if (darkModeEnabled) {
                        item->setBackground(QColor("#546E7A"));  // Dark mode highlight
                    } else {
                        item->setBackground(QColor("#FFF9C4"));  // Light mode highlight
                    }
                } else {
                    // Reset background for non-sorted columns
                    item->setBackground(QBrush(Qt::transparent));
                }

                // Preserve Income/Expense colors in the Type column (column 5)
                if (col == 5) {
                    QString type = item->text();
                    if (type == "Income") {
                        item->setForeground(QColor("#00C853"));  // Darker green for Income
                    } else if (type == "Expense") {
                        item->setForeground(QColor("#FF5252"));  // Subtle red for Expense
                    }
                } else {
                    // Apply default text color based on mode for other columns
                    item->setForeground(QBrush(darkModeEnabled ? Qt::white : Qt::black));
                }
            }
        }
    }
}





