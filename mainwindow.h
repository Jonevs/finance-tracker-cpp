#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QShortcut>
#include <QMap>
#include "customtablewidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addTransaction();    
    void editTransaction();  
    void deleteTransaction(); 
    void onTransactionSelected(); 
    void loadTransactions();  
    void clearForm(); 
    void applyFilters(); 
    void clearFilters(); 
    void exportToCSV();
    void sortTable(int column);
    void updateHeaderArrows(int sortedColumn, bool ascending);
    void highlightSortedColumn();
    void clearSorting();
    void updateTableColors();

private:
    void setupUI();

    // Form Inputs
    QLineEdit *descriptionInput;
    QComboBox *categoryInput;
    QComboBox *typeInput; 
    QLineEdit *amountInput;
    QDateEdit *dateInput;
    QPushButton *addButton;
    QPushButton *editButton;    
    QPushButton *deleteButton; 

    // Table 
    CustomTableWidget *transactionTable;

    // Filter & Search Elements
    QLineEdit *searchInput;        
    QComboBox *filterCategory;     
    QComboBox *filterType;         
    QDateEdit *filterStartDate;    
    QDateEdit *filterEndDate;    
    QShortcut *sortByDateShortcut;
    QShortcut *sortByAmountShortcut;  
    QMap<int, bool> columnSortOrder;

    int selectedTransactionId = -1;
    int lastSelectedRow = -1;     
    int currentSortedColumn = -1;  
 
    bool dateSortAscending = true;    
    bool amountSortAscending = true;  

    bool darkModeEnabled = false; 

    QPushButton* createStyledButton(const QString &text, const QString &color, const QString &hoverColor, bool disabled = false);
    QComboBox* createComboBox(const QStringList &items);
    QDateEdit* createDateEdit(const QDate &date); 
    QString getDarkModeStyle();
};

#endif // MAINWINDOW_H
