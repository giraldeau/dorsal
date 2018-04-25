#include <QDebug>
#include <QVector>

#include <cmath>
#include <cstdio>

int idx(double xmin, double res, double x) {
    return std::floor((x - xmin) / res);
}

int main()
{
    QVector<int> v({1, 2, 3, 4, 5});

    /* when i overflows, stop iteration */
    for (uint i = v.length() - 1; i < (uint) v.length(); i--) {
        //qDebug() << v[i];
    }

    QVector<int> result(v.length());

    double xmin = -0.5e10;
    double xmax = 1.1e6;
    double res = (xmax - xmin) / v.length();
    uint n = 100000;
    double step = (xmax - xmin) / n;
    qDebug() << "reso" << res;
    qDebug() << "step" << step;
    for (uint i = 0; i < n; i++) {
        uint index = idx(xmin, res, xmin + (i * step));
        //qDebug() << index;
        result[index]++;
    }

    qDebug() << result;

    return 0;
}

