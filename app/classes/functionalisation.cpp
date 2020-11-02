#include "functionalisation.h"

Functionalisation::Functionalisation()
{

}

Functionalisation::Functionalisation(size_t size, int value):
    vector(std::vector<int>(size, value))
{}

QString Functionalisation::getName()
{
    // check if custom functionalisation was set
    if (name == "None")
    {
        for (size_t i=0; i<vector.size(); i++)
        {
            if (vector[i] != 0)
            {
                name = "Custom";
                break;
            }
        }
    }
    return name;
}

int &Functionalisation::operator[](int i)
{
    return vector[i];
}

int Functionalisation::operator[](int i) const
{
    return vector[i];
}

bool Functionalisation::operator==(const Functionalisation &other) const
{
    for (int i=0; i<size(); i++)
    {
        if ((*this)[i] != other[i])
            return false;
    }
    return true;
}

bool Functionalisation::operator!=(const Functionalisation &other) const
{
    return !(*this == other);
}

QMap<int, int> Functionalisation::getFuncMap() const
{
    return getFuncMap(std::vector<bool>(size(), false));
}

QMap<int, int> Functionalisation::getFuncMap(const std::vector<bool> &sensorFailures) const
{
    Q_ASSERT(vector.size() == sensorFailures.size());

    QMap<int, int> funcMap;
    for (int i=0; i<size(); i++)
    {
        int func = vector[i];
        if (!funcMap.contains(func))
            // sensor failure in channel i:
            // assign 0, so failing funcs are not overseen
            funcMap[func] = sensorFailures[i] ? 0 : 1;
        else
            if (!sensorFailures[i])
                funcMap[func]++;
    }
    return funcMap;
}

int Functionalisation::getNFuncs() const
{
    return getFuncMap().size();
}

int Functionalisation::size() const
{
    return vector.size();
}

void Functionalisation::setVector(const std::vector<int> &value)
{
    vector = value;
}

void Functionalisation::setName(const QString &value)
{
    name = value;
}
