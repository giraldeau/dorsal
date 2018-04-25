#include <iostream>
#include <memory>
#include <vector>

#include <QScopedPointer>
#include <QVector>
#include <QDebug>

using namespace std;

struct Object {
    Object() { qDebug() << "Object" << this; }
    virtual ~Object() { qDebug() << "~Object" << this; }
    int arr[1000];
};

struct ItemOne {
    ItemOne(Object *obj) : m_obj(obj)
    { qDebug() << "ItemOne" << this; }
    virtual ~ItemOne()
    { qDebug() << "~ItemOne" << this; }
    unique_ptr<Object> m_obj;
};

int main()
{
    //ItemOne one(new Object);

    /*
    vector<unique_ptr<ItemOne>> v;
    v.push_back(make_unique<ItemOne>(new Object));

    try {
        for (unique_ptr<ItemOne> &item : v) {
            for (int i = 0; i < 100; i++) {
                item->m_obj->arr[i] = i;
                if ((i % 50) == 0)
                    throw i;
            }
        }
    } catch (...) {
        qDebug() << "exception";
    }
    */
    qDebug() << "done";
}
