/*
 * Copyright (c) 2014-2026 Patrizio Bekerle -- <patrizio@bekerle.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */

#include "layoutdialog.h"

#include <services/settingsservice.h>
#include <utils/gui.h>
#include <utils/misc.h>

#include <QInputDialog>
#include <QMessageBox>

#include "ui_layoutdialog.h"

LayoutDialog::LayoutDialog(QWidget *parent) : MasterDialog(parent), ui(new Ui::LayoutDialog) {
    ui->setupUi(this);
    afterSetupUI();
    reloadLayoutList();
}

LayoutDialog::~LayoutDialog() { delete ui; }

/**
 * Reloads the layout list widget from settings
 */
void LayoutDialog::reloadLayoutList() {
    SettingsService settings;
    const QStringList uuids = settings.value(QStringLiteral("layouts")).toStringList();
    const QString currentUuid = settings.value(QStringLiteral("currentLayout")).toString();

    ui->layoutListWidget->clear();

    for (const QString &uuid : uuids) {
        const QString name =
            settings.value(QStringLiteral("layout-") + uuid + QStringLiteral("/name")).toString();

        auto *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, uuid);
        ui->layoutListWidget->addItem(item);

        if (uuid == currentUuid) {
            ui->layoutListWidget->setCurrentItem(item);
        }
    }

    updateButtonStates();
}

/**
 * Updates button enabled states based on current selection and list size
 */
void LayoutDialog::updateButtonStates() {
    const int count = ui->layoutListWidget->count();
    const int currentRow = ui->layoutListWidget->currentRow();
    const bool hasSelection = currentRow >= 0;

    // Cannot remove the last remaining layout
    ui->removeButton->setEnabled(hasSelection && count > 1);
    ui->renameButton->setEnabled(hasSelection);
    ui->moveUpButton->setEnabled(hasSelection && currentRow > 0);
    ui->moveDownButton->setEnabled(hasSelection && currentRow < count - 1);
}

/**
 * Persists the current order of layouts in the list widget to settings
 */
void LayoutDialog::persistLayoutOrder() {
    QStringList uuids;
    uuids.reserve(ui->layoutListWidget->count());

    for (int i = 0; i < ui->layoutListWidget->count(); ++i) {
        uuids << ui->layoutListWidget->item(i)->data(Qt::UserRole).toString();
    }

    SettingsService settings;
    settings.setValue(QStringLiteral("layouts"), uuids);
}

/**
 * Adds a new layout
 */
void LayoutDialog::on_addButton_clicked() {
    const QString name =
        QInputDialog::getText(this, tr("Add layout"), tr("Layout name:")).trimmed();

    if (name.isEmpty()) {
        return;
    }

    SettingsService settings;
    const QString uuid = Utils::Misc::createUuidString();
    QStringList uuids = settings.value(QStringLiteral("layouts")).toStringList();
    uuids.append(uuid);

    settings.setValue(QStringLiteral("layouts"), uuids);
    settings.setValue(QStringLiteral("layout-") + uuid + QStringLiteral("/name"), name);

    reloadLayoutList();

    // Select the newly added item
    for (int i = 0; i < ui->layoutListWidget->count(); ++i) {
        if (ui->layoutListWidget->item(i)->data(Qt::UserRole).toString() == uuid) {
            ui->layoutListWidget->setCurrentRow(i);
            break;
        }
    }
}

/**
 * Removes the selected layout
 */
void LayoutDialog::on_removeButton_clicked() {
    QListWidgetItem *item = ui->layoutListWidget->currentItem();
    if (item == nullptr) {
        return;
    }

    if (ui->layoutListWidget->count() < 2) {
        return;
    }

    if (Utils::Gui::question(this, tr("Remove layout"), tr("Remove the selected layout?"),
                             QStringLiteral("remove-layout")) != QMessageBox::Yes) {
        return;
    }

    const QString uuid = item->data(Qt::UserRole).toString();
    SettingsService settings;
    QStringList uuids = settings.value(QStringLiteral("layouts")).toStringList();
    uuids.removeAll(uuid);
    settings.setValue(QStringLiteral("layouts"), uuids);

    // Remove all settings for this layout
    settings.beginGroup(QStringLiteral("layout-") + uuid);
    settings.remove(QLatin1String(""));
    settings.endGroup();

    // Switch away if this was the current layout
    const QString currentUuid = settings.value(QStringLiteral("currentLayout")).toString();
    if (currentUuid == uuid) {
        settings.setValue(QStringLiteral("currentLayout"),
                          uuids.isEmpty() ? QString() : uuids.at(0));
    }

    reloadLayoutList();
}

/**
 * Renames the selected layout
 */
void LayoutDialog::on_renameButton_clicked() {
    QListWidgetItem *item = ui->layoutListWidget->currentItem();
    if (item == nullptr) {
        return;
    }

    const QString uuid = item->data(Qt::UserRole).toString();
    SettingsService settings;
    const QString oldName =
        settings.value(QStringLiteral("layout-") + uuid + QStringLiteral("/name")).toString();

    const QString newName = QInputDialog::getText(this, tr("Rename layout"), tr("Layout name:"),
                                                  QLineEdit::Normal, oldName)
                                .trimmed();

    if (newName.isEmpty() || newName == oldName) {
        return;
    }

    settings.setValue(QStringLiteral("layout-") + uuid + QStringLiteral("/name"), newName);

    item->setText(newName);
    updateButtonStates();
}

/**
 * Moves the selected layout up in the list
 */
void LayoutDialog::on_moveUpButton_clicked() {
    const int row = ui->layoutListWidget->currentRow();
    if (row <= 0) {
        return;
    }

    QListWidgetItem *item = ui->layoutListWidget->takeItem(row);
    ui->layoutListWidget->insertItem(row - 1, item);
    ui->layoutListWidget->setCurrentRow(row - 1);

    persistLayoutOrder();
    updateButtonStates();
}

/**
 * Moves the selected layout down in the list
 */
void LayoutDialog::on_moveDownButton_clicked() {
    const int row = ui->layoutListWidget->currentRow();
    if (row < 0 || row >= ui->layoutListWidget->count() - 1) {
        return;
    }

    QListWidgetItem *item = ui->layoutListWidget->takeItem(row);
    ui->layoutListWidget->insertItem(row + 1, item);
    ui->layoutListWidget->setCurrentRow(row + 1);

    persistLayoutOrder();
    updateButtonStates();
}

/**
 * Updates button states when the selection changes
 */
void LayoutDialog::on_layoutListWidget_currentRowChanged(int currentRow) {
    Q_UNUSED(currentRow)
    updateButtonStates();
}

/**
 * Opens the rename dialog on double click
 */
void LayoutDialog::on_layoutListWidget_itemDoubleClicked() { on_renameButton_clicked(); }
