/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Random.h"
#include <QSharedPointer>
#include <sodium.h>
#include <QDebug>

QSharedPointer<Random> Random::m_instance;

QSharedPointer<Random> Random::instance()
{
    if (!m_instance) {
        m_instance.reset(new Random());
    }
    return m_instance;
}

Random::Random()
{
}

quint32 Random::randomUInt(quint32 limit)
{
    Q_ASSERT(limit <= QUINT32_MAX);
    if (limit == 0) {
        return 0;
    }

    quint32 rand;
    const quint32 ceil = QUINT32_MAX - (QUINT32_MAX % limit) - 1;

    if (sodium_init() < 0)
    {
        qDebug() << "sodium init problem.";
        return 0;
    }
    else
    {
        rand = randombytes_uniform(ceil);
    }

    return (rand % limit);
}
