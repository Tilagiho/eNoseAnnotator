#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "../app/mvector.h"

class TestENoseAnnotator : public QObject
{
    Q_OBJECT

public:
    TestENoseAnnotator();
    ~TestENoseAnnotator();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_mvector();

};

TestENoseAnnotator::TestENoseAnnotator()
{

}

TestENoseAnnotator::~TestENoseAnnotator()
{

}

void TestENoseAnnotator::initTestCase()
{

}

void TestENoseAnnotator::cleanupTestCase()
{

}

void TestENoseAnnotator::test_mvector()
{
    MVector vector;

    // test init
    bool test = true;
    for (int i=0; i<MVector::size; i++)
        if (vector.array[i] != 0.0)
            test = false;
    QVERIFY(test);

    // test == and !=
    MVector vectorZero, vectorNotZero;

    vectorNotZero.array[MVector::size/2] = 0.1;

    QVERIFY (vectorZero != vectorNotZero);
    QVERIFY (! (vectorZero == vectorNotZero));
    QVERIFY (vector == vectorZero);
}

QTEST_MAIN(TestENoseAnnotator)

#include "tst_enoseannotator.moc"
