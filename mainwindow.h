#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>

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

private:
    QLineEdit *descriptionInput;
    QComboBox *categoryInput;
    QLineEdit *amountInput;
    QDateEdit *dateInput;
    QPushButton *addButton;
    QPushButton *editButton;    
    QPushButton *deleteButton;  
    QTableWidget *transactionTable;

    int selectedTransactionId;  

    void setupUI();          
    void loadTransactions();  
    void clearForm();        
};

#endif // MAINWINDOW_H
