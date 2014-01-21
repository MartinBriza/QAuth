/*
 * Test valid communication between the helper and the library
 * Copyright (C) 2014 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "DataTransfer.h"

#include <QAuth.h>

#include <QtTest>

QTEST_MAIN(DataTransfer);

void DataTransfer::initTestCase() {

}

void DataTransfer::cleanupTestCase() {

}

void DataTransfer::init() {

}

void DataTransfer::cleanup() {

}

void DataTransfer::basicTest() {
    QAuth qauth;
    QEventLoop loop(&qauth);
    qauth.setVerbosity(true);
    qauth.start();
    loop.exec();
}

#include "DataTransfer.moc"
