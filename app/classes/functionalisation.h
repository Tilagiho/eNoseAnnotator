#ifndef FUNCTIONALISATION_H
#define FUNCTIONALISATION_H

#include <QString>
#include <vector>
#include <QMap>

class Functionalisation
{
public:
    Functionalisation();
    Functionalisation(size_t size, int value);

    QString getName();

    int &operator[] (int index);
    int operator[] (int index) const;

    bool operator ==(const Functionalisation &other) const;
    bool operator !=(const Functionalisation &other) const;

    QMap<int, int> getFuncMap() const;
    QMap<int, int> getFuncMap(const std::vector<bool> &sensorFailures) const;

    int getNFuncs() const;

    int size() const;

    void setVector(const std::vector<int> &value);

    void setName(const QString &value);

private:
    QString name = "None";
    std::vector<int> vector;
};

#endif // FUNCTIONALISATION_H
