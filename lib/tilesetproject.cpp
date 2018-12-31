/*
    Copyright 2017, Mitch Curtis

    This file is part of Slate.

    Slate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Slate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Slate. If not, see <http://www.gnu.org/licenses/>.
*/

#include "tilesetproject.h"

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QPainter>
#include <QUndoStack>

#include "changetilecanvassizecommand.h"
#include "jsonutils.h"
#include "utils.h"

TilesetProject::TilesetProject() :
    Project(),
    mTilesWide(0),
    mTilesHigh(0),
    mTileMapImage(),
    mTileset(nullptr)
{
    setObjectName(QLatin1String("tilesetProject"));

    qCDebug(lcProjectLifecycle) << "constructing" << this;
}

TilesetProject::~TilesetProject()
{
    qCDebug(lcProjectLifecycle) << "destructing" << this;
}

QAbstractItemModel *TilesetProject::tileSetModel() const
{
    return mTileset;
}

Project::Type TilesetProject::type() const
{
    return TilesetType;
}

void TilesetProject::createTilesetTiles(int tilesetTilesWide, int tilesetTilesHigh)
{
    Q_ASSERT(mTileDatabase.isEmpty());
    for (int row = 0; row < tilesetTilesHigh; ++row) {
        for (int column = 0; column < tilesetTilesWide; ++column) {
            const QPoint tileCoord = QPoint(column, row);
            const int index = mTileset->indexFromTileCoord(tileCoord);
            mTileDatabase.insert(index, new TileObject(index, mTileset, mTileset->tileCoordRect(tileCoord)));
        }
    }
    Q_ASSERT(!mTileDatabase.isEmpty());
}

void TilesetProject::createNew(QUrl tilesetUrl, int tileWidth, int tileHeight,
    int tilesetTilesWide, int tilesetTilesHigh,
    int canvasTilesWide, int canvasTilesHigh, bool transparentBackground)
{
    if (hasLoaded()) {
        close();
    }

    if (tilesetUrl.isEmpty()) {
        // Not using an existing tileset image, so we must create one.
        tilesetUrl = createTemporaryImage(tilesetTilesWide * tileWidth, tilesetTilesHigh * tileHeight,
            transparentBackground ? Qt::transparent : Qt::white);
        if (!tilesetUrl.isValid()) {
            return;
        }
    }

    QImage tilesetImage(tilesetUrl.toLocalFile());
    if (tileWidth * tilesetTilesWide > tilesetImage.width()) {
        error(QString::fromLatin1("Tile width of %1 pixels combined with a tileset width "
            "of %2 tiles is too large for a tileset image whose width is %3 pixels")
                .arg(tileWidth).arg(tilesetTilesWide).arg(tilesetImage.width()));
        return;
    }

    if (tileHeight * tilesetTilesHigh > tilesetImage.height()) {
        error(QString::fromLatin1("Tile height of %1 pixels combined with a tileset height "
            "of %2 tiles is too large for a tileset image whose height is %3 pixels")
                .arg(tileHeight).arg(tilesetTilesHigh).arg(tilesetImage.height()));
        return;
    }

    qCDebug(lcProject) << "creating new project:"
        << "tileWidth =" << tileWidth
        << "tileHeight =" << tileHeight
        << "tilesetTilesWide =" << tilesetTilesWide
        << "tilesetTilesHigh =" << tilesetTilesHigh
        << "canvasTilesWide =" << canvasTilesWide
        << "canvasTilesHigh =" << canvasTilesHigh
        << "tilesetPath =" << tilesetUrl;

    Q_ASSERT(mUndoStack.count() == 0);
    Q_ASSERT(!mTileset);

    setTilesWide(canvasTilesWide);
    setTilesHigh(canvasTilesHigh);
    setTilesetUrl(tilesetUrl);
    const QImage image(tilesetUrl.toLocalFile());
    const QSize tileSize(image.width() / tilesetTilesWide, image.height() / tilesetTilesHigh);
    setTileset(new Tileset(tilesetUrl.toLocalFile(), image, tileSize, tilesetTilesWide, tilesetTilesWide * tilesetTilesHigh, this));

    createTilesetTiles(tilesetTilesWide, tilesetTilesHigh);

    mTiles.clear();
    mTiles.fill(-1, mTilesWide * mTilesHigh);

    mTileMapImage = QImage(QSize(canvasTilesWide, canvasTilesHigh), TileMapImageFormat);
    mTileMapImage.fill(TileIndexInvalid);

    setUrl(QUrl());
    setNewProject(true);
    emit projectCreated();

    qCDebug(lcProject) << "finished creating new project";
}

void TilesetProject::doLoad(const QUrl &url)
{
    QFile jsonFile(url.toLocalFile());
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        error(QString::fromLatin1("Failed to open tileset project's STP file at %1").arg(url.toLocalFile()));
        return;
    }

    if (QFileInfo(jsonFile).suffix() != "stp") {
        error(QString::fromLatin1("Tileset project files must have a .stp extension (%1)").arg(url.toLocalFile()));
        return;
    }

    qDebug() << url.toLocalFile();//////////////////////////////
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject rootJson = jsonDoc.object();
    QJsonObject projectObject = JsonUtils::strictValue(rootJson, "project").toObject();

    setTilesWide(JsonUtils::strictValue(projectObject, "tilesWide").toInt());
    setTilesHigh(JsonUtils::strictValue(projectObject, "tilesHigh").toInt());
    setTilesetUrl(QUrl::fromLocalFile(JsonUtils::strictValue(projectObject, "tilesetPath").toString()));
    mUsingTempImage = false;

    Q_ASSERT(!mTileset);
    const QString tilesetPath = mTilesetUrl.toLocalFile();
    QJsonObject tilesetObject = JsonUtils::strictValue(projectObject, "tileset").toObject();
    const int tilesetTilesWide = JsonUtils::strictValue(tilesetObject, "tilesWide").toInt();
    const int tilesetTilesHigh = JsonUtils::strictValue(tilesetObject, "tilesHigh").toInt();

    const QImage image(tilesetPath);
    const QSize tileSize(image.width() / tilesetTilesWide, image.height() / tilesetTilesHigh);
    Tileset *tempTileset = new Tileset(tilesetPath, image, tileSize, tilesetTilesWide, tilesetTilesWide * tilesetTilesHigh, this);

//    Tileset *tempTileset = new Tileset(tilesetPath, tilesetTilesWide, tilesetTilesHigh, this);
    if (tempTileset->isValid()) {
        setTileset(tempTileset);
    } else {
        error(QString::fromLatin1("Failed to open project's tileset at %1").arg(tilesetPath));
        return;
    }

    mTileDatabase.clear();
    createTilesetTiles(tilesetTilesWide, tilesetTilesHigh);

    QJsonArray tileArray = projectObject["tiles"].toArray();
    mTiles.resize(tileArray.size());
    for (int i = 0; i < tileArray.size(); ++i) {
        int tileId = tileArray.at(i).toInt(-2);
        Q_ASSERT(tileId != -2);
        mTiles[i] = tileId;
        if (tileId > -1) {
//            Q_ASSERT(mTileDatabase.contains(tileId));
        }
    }

    readGuides(projectObject);
    mCachedProjectJson = projectObject;

    setUrl(url);
    emit projectLoaded();
}

void TilesetProject::doClose()
{
    setNewProject(false);
    setUrl(QUrl());
    mUsingTempImage = false;
    clearTiles();
    mTileDatabase.clear();
    setTileset(nullptr);
    mUndoStack.clear();
    emit projectClosed();
}

void TilesetProject::doSaveAs(const QUrl &url)
{
    if (!hasLoaded())
        return;

    if (url.isEmpty())
        return;

    const QString filePath = url.toLocalFile();
    const QFileInfo projectSaveFileInfo(filePath);
    if (mTempDir.isValid()) {
        if (projectSaveFileInfo.dir().path() == mTempDir.path()) {
            error(QLatin1String("Cannot save project in internal temporary directory"));
            return;
        }
    }

    if (mTileset->image().isNull()) {
        error(QString::fromLatin1("Failed to save project: tileset image is null"));
        return;
    }

    QFile jsonFile;
    if (QFile::exists(filePath)) {
        jsonFile.setFileName(filePath);
        if (!jsonFile.open(QIODevice::WriteOnly)) {
            error(QString::fromLatin1("Failed to open project's JSON file at %1").arg(filePath));
            return;
        }
    } else {
        jsonFile.setFileName(filePath);
        if (!jsonFile.open(QIODevice::WriteOnly)) {
            error(QString::fromLatin1("Failed to create project's JSON file at %1").arg(filePath));
            return;
        }
    }

    const QFileInfo tileFileInfo(mTileset->fileName());
    if (!tileFileInfo.exists()) {
        error(QString::fromLatin1("Failed to save project: tileset path %1 doesn't exist").arg(mTileset->fileName()));
        return;
    }

    if (mUsingTempImage) {
        // Save the image in the same directory as the project, using the same base name as the project url.
        const QString path = projectSaveFileInfo.path() + "/" + projectSaveFileInfo.completeBaseName() + ".png";
        qCDebug(lcProject) << "saving temporary tileset image to" << path;
        if (!mTileset->image().save(path)) {
            error(QString::fromLatin1("Failed to save project: failed to save tileset image %1").arg(path));
            return;
        }

        mTileset->setFileName(path);
        mTilesetUrl = QUrl::fromLocalFile(path);
        mUsingTempImage = false;
    } else {
        if (!mTileset->image().save(mTileset->fileName())) {
            error(QString::fromLatin1("Failed to save project: failed to save tileset image %1").arg(mTileset->fileName()));
            return;
        }
    }

    QJsonObject rootJson;

    QJsonObject projectObject;
    projectObject["tilesWide"] = mTilesWide;
    projectObject["tilesHigh"] = mTilesHigh;
    projectObject["tileWidth"] = tileSize().width();
    projectObject["tileHeight"] = tileSize().height();
    projectObject["tilesetPath"] = mTilesetUrl.toLocalFile();

    QJsonObject tilesetObject;
    tilesetObject["tilesWide"] = mTileset->imageSizeTiles().width();
    tilesetObject["tilesHigh"] = mTileset->imageSizeTiles().height();
    projectObject.insert("tileset", tilesetObject);

    QJsonArray tileArray;
    foreach (int tile, mTiles) {
        tileArray.append(QJsonValue(tile));
    }
    projectObject.insert("tiles", tileArray);

    writeGuides(projectObject);
    emit readyForWritingToJson(&projectObject);

    rootJson.insert("project", projectObject);

    QJsonDocument jsonDoc(rootJson);
    const qint64 bytesWritten = jsonFile.write(jsonDoc.toJson());
    if (bytesWritten == -1) {
        error(QString::fromLatin1("Failed to save project: couldn't write to JSON project file: %1")
            .arg(jsonFile.errorString()));
        return;
    }

    if (bytesWritten == 0) {
        error(QString::fromLatin1("Failed to save project: wrote zero bytes to JSON project file"));
        return;
    }

    if (mFromNew) {
        // The project was successfully saved, so it can now save
        // to the same URL by default from now on.
        setNewProject(false);
    }
    setUrl(url);
    mUndoStack.setClean();
    mHadUnsavedChangesBeforeMacroBegan = false;
}

int TilesetProject::tileIdFromPosInTileset(int x, int y) const
{
    const int column = Utils::divFloor(x, tileSize().width());
    const int row = Utils::divFloor(y, tileSize().height());
    return tileIdFromTilePosInTileset(column, row);
}

int TilesetProject::tileIdFromTilePosInTileset(int column, int row) const
{
    // IDs are one-based.
    return (row * tileset()->tileSize().width() + column);
}

void TilesetProject::changeSize(const QSize &newSize, const QVector<int> &tiles)
{
    Q_ASSERT(newSize != size());

    // Size < 1 doesn't make sense.
    Q_ASSERT(newSize.width() >= 1 && newSize.height() >= 1);

    // This function can be called as a result of the size changing or a size
    // change being undone. When the size shrinks, we never need a list of tiles.
    // When the size increases, we only need a list of tiles if the size was changed
    // as a result of an undo operation, as we need to restore those tiles.
    Q_ASSERT(tiles.isEmpty() || newSize.width() * newSize.height() == tiles.size());

    if (tiles.isEmpty()) {
        // Expand the 1d array into a 2d one, then simply resize the 2d one
        // and flatten back again afterwards.
        // Probably the slowest way of doing it, but it doesn't try to be tricky,
        // which makes it easier to understand and maintain.
        // Also, resizing shouldn't happen that often, so it doesn't matter.
        QVector<QVector<int> > tileGrid;
        tileGrid.resize(mTilesHigh);
        for (int row = 0; row < mTilesHigh; ++row) {
            tileGrid[row].resize(mTilesWide);

            for (int column = 0; column < mTilesWide; ++column) {
                const int index = row * mTilesWide + column;
                tileGrid[row][column] = mTiles.at(index);
            }
        }

        // First, shrink or expand the 2d grid's height to match the new height.
        if (newSize.height() < mTilesHigh) {
            while (tileGrid.size() > newSize.height())
                tileGrid.removeLast();
        } else {
            while (tileGrid.size() < newSize.height()) {
                QVector<int> row;
                row.fill(TileObject::invalidId(), newSize.width());
                tileGrid.append(row);
            }
        }

        // Do the same for the width.
        if (newSize.width() < mTilesWide) {
            for (int row = 0; row < newSize.height(); ++row) {
                while (tileGrid[row].size() > newSize.width()) {
                    tileGrid[row].removeLast();
                }
            }
        } else {
            for (int row = 0; row < newSize.height(); ++row) {
                while (tileGrid[row].size() < newSize.width()) {
                    tileGrid[row].append(TileObject::invalidId());
                }
            }
        }

        mTiles.resize(newSize.width() * newSize.height());

        for (int row = 0; row < newSize.height(); ++row) {
            for (int column = 0; column < newSize.width(); ++column) {
                const int index = row * newSize.width() + column;
                mTiles[index] = tileGrid.at(row).at(column);
            }
        }
    } else {
        mTiles = tiles;
    }

    QImage newTileMapImage(newSize, TileMapImageFormat);
    newTileMapImage.fill(TileIndexInvalid);
    QPainter painter(&newTileMapImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(QPoint(0, 0), mTileMapImage, mTileMapImage.rect(), Qt::ImageConversionFlag::NoFormatConversion);

    setTilesWide(newSize.width());
    setTilesHigh(newSize.height());
}

int TilesetProject::tilesWide() const
{
    return mTilesWide;
}

void TilesetProject::setTilesWide(int tilesWide)
{
    if (tilesWide == mTilesWide)
        return;

    mTilesWide = tilesWide;
    emit tilesWideChanged();
    emit sizeChanged();
}

int TilesetProject::tilesHigh() const
{
    return mTilesHigh;
}

void TilesetProject::setTilesHigh(int tilesHigh)
{
    if (tilesHigh == mTilesHigh)
        return;

    mTilesHigh = tilesHigh;
    emit tilesHighChanged();
    emit sizeChanged();
}

QSize TilesetProject::tileSize() const
{
    return mTileset ? mTileset->tileSize() : QSize();
}

QSize TilesetProject::pixelSize() const
{
    return QSize(mTilesWide * tileSize().width(), mTilesHigh * tileSize().height());
}

void TilesetProject::resize(const QSize &newSize)
{
    if (newSize == size())
        return;

    beginMacro(QLatin1String("ChangeCanvasSize"));
    addChange(new ChangeTileCanvasSizeCommand(this, QSize(mTilesWide, mTilesHigh), newSize));
    endMacro();
}

QSize TilesetProject::size() const
{
    return QSize(mTilesWide, mTilesHigh);
}

QRect TilesetProject::bounds() const
{
    return QRect(QPoint(0, 0), pixelSize());
}

QRect TilesetProject::tileBounds() const
{
    return QRect(QPoint(0, 0), size());
}

QImage TilesetProject::exportedImage() const
{
    // Although other projects would return what's visible on the canvas,
    // tileset projects are a bit different in that the tileset is what's being
    // drawn on, so we're more interested in that than the collection of tiles
    // that the user has drawn onto the canvas, which is more a preview.
    return mTileset->image();
}

QUrl TilesetProject::tilesetUrl() const
{
    return mTilesetUrl;
}

void TilesetProject::setTilesetUrl(const QUrl &tilesetUrl)
{
    if (tilesetUrl == mTilesetUrl)
        return;

    mTilesetUrl = tilesetUrl;
    emit tilesetUrlChanged();
}

Tileset *TilesetProject::tileset() const
{
    return mTileset;
}

void TilesetProject::setTileset(Tileset *tileset)
{
    if (tileset == mTileset)
        return;

    Tileset *old = mTileset;
    mTileset = tileset;
    emit tilesetChanged(old, mTileset);
    emit tileSizeChanged();
    emit tileSetModelChanged(mTileset);
}

const TileObject *TilesetProject::tileObjectAtPixel(const QPoint &pixel) const
{
    if (!bounds().contains(pixel)) {
        return nullptr;
    }

    const QPoint tilePos(Utils::divFloor(pixel.x(), tileSize().width()), Utils::divFloor(pixel.y(), tileSize().height()));
    return tileObjectAt(tilePos);
}

TileObject *TilesetProject::tileObjectAtPixel(const QPoint &pixel)
{
    return const_cast<TileObject*>(static_cast<const TilesetProject*>(this)->tileObjectAtPixel(pixel));
}

int TilesetProject::tile(const QPoint &tileCoord) const
{
    if (!tileBounds().contains(tileCoord))
        return -1;

    const int flatIndex = tileCoord.y() * mTilesWide + tileCoord.x();
    Q_ASSERT(flatIndex < mTiles.size());
    return mTiles[flatIndex];
//    return mTileMapImage.pixel(tilePos);
}

void TilesetProject::setTile(const QPoint &tileCoord, const int index)
{
    if(!tileBounds().contains(tileCoord))
        return;

    const int flatIndex = tileCoord.y() * mTilesWide + tileCoord.x();
    Q_ASSERT(flatIndex < mTiles.size());
    mTiles[flatIndex] = index;
    //    mTileMapImage.setPixel(tilePos, index);
}

QVector<int> TilesetProject::tiles() const
{
    return mTiles;
}

const TileObject *TilesetProject::tileObjectAt(const QPoint &tilePos) const
{
    const int index = tile(tilePos);
    return mTileDatabase.value(index);
}

TileObject *TilesetProject::tilesetTileAt(int xInPixels, int yInPixels)
{
    Tileset *set = tileset();
    if (!set) {
        return nullptr;
    }

    if (xInPixels < 0 || xInPixels >= set->image().width()
        || yInPixels < 0 || yInPixels >= set->image().height()) {
        // qWarning() << Q_FUNC_INFO << xInPixels << "," << yInPixels << "is out of tileset swatch bounds";
        return nullptr;
    }

    return mTileDatabase.value(tileIdFromPosInTileset(xInPixels, yInPixels));
}

TileObject *TilesetProject::tilesetTileAtTilePos(const QPoint &tilePos) const
{
    Tileset *set = tileset();
    if (!set) {
        return nullptr;
    }

    if (tilePos.x() < 0 || tilePos.x() >= set->imageSizeTiles().width()
        || tilePos.y() < 0 || tilePos.y() >= set->imageSizeTiles().height()) {
        // qWarning() << Q_FUNC_INFO << tileX << "," << tileY << "is out of tileset swatch bounds";
        return nullptr;
    }

    return mTileDatabase.value(tileIdFromTilePosInTileset(tilePos.x(), tilePos.y()));
}

TileObject *TilesetProject::tilesetTileAtId(int id)
{
    Tileset *set = tileset();
    if (!set) {
        return nullptr;
    }

    QHash<int, TileObject*>::iterator it = mTileDatabase.find(id);
    if (it == mTileDatabase.end())
        return nullptr;

    return it.value();
}

TileObject TilesetProject::tileObjectForIndex(const int index)
{
    const QPoint tileCoord = mTileset->tileCoordFromIndex(index);
    const int x = tileCoord.x() * tileSize().width();
    const int y = tileCoord.y() * tileSize().height();
    return TileObject(index, mTileset, mTileset->tileCoordRect(tileCoord));
}

QPoint TilesetProject::tileIdToTilePos(int tileId) const
{
    return QPoint(tileId % mTileset->tileSize().width(), tileId / mTileset->tileSize().width());
}

void TilesetProject::clearTiles()
{
    if (mTiles.isEmpty())
        return;

    mTiles.fill(mTiles.size(), -1);
    emit tilesCleared();

    mTileMapImage.fill(TileIndexInvalid);
}
