#ifndef CRUD_H
#define CRUD_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QReadWriteLock>

class Crud : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOpen READ getIsOpen NOTIFY isOpenChanged)

    Crud                (const Crud &) noexcept;
    Crud &operator =    (const Crud &) noexcept;
    explicit Crud(QObject *parent = nullptr) noexcept;
    ~Crud() noexcept;
    static Crud         *m_Crud;
public:
    static Crud *getInstance();
    static void drop();
    void openDatabase();
    void closeDatabase();
    bool getIsOpen() const;
    QSqlQuery executeQuery(const QString &sql);
signals:
    void isOpenChanged();
private:
    QSqlDatabase m_Database{};
    mutable QReadWriteLock mutex{};
    bool m_isOpen{false};
};

#endif // CRUD_H
