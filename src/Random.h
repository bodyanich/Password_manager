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

#ifndef RANDOM_H
#define RANDOM_H

#include <QSharedPointer>

#ifndef QUINT32_MAX
#define QUINT32_MAX 4294967295U
#endif

class Random
{
public:
    static QSharedPointer<Random> instance();

    /**
     * Generate a random quint32 in the range [0, @p limit)
     */
    quint32 randomUInt(quint32 limit);


private:
    explicit Random();
    Q_DISABLE_COPY(Random);

    static QSharedPointer<Random> m_instance;
};

static inline QSharedPointer<Random> randomGen()
{
    return Random::instance();
}

#endif
