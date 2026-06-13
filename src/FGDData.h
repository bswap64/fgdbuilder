#pragma once
#include <QString>
#include <QStringList>
#include <QList>

enum class EntityClassType {
    PointClass,
    SolidClass,
    NPCClass,
    KeyFrameClass,
    MoveClass,
    FilterClass,
    BaseClass
};

inline QString classTypeToString(EntityClassType t) {
    switch (t) {
        case EntityClassType::PointClass:    return "@PointClass";
        case EntityClassType::SolidClass:    return "@SolidClass";
        case EntityClassType::NPCClass:      return "@NPCClass";
        case EntityClassType::KeyFrameClass: return "@KeyFrameClass";
        case EntityClassType::MoveClass:     return "@MoveClass";
        case EntityClassType::FilterClass:   return "@FilterClass";
        case EntityClassType::BaseClass:     return "@BaseClass";
    }
    return "@PointClass";
}

struct FGDKeyvalue {
    QString name;
    QString type;
    QString displayName;
    QString defaultValue;
    QString description;
    bool readOnly = false;
    bool report = false;
    QList<QPair<QString,QString>> choiceItems;
    QList<QPair<QString,QString>> choiceDescriptions;
    QList<QPair<QString, QPair<QString,int>>> flagItems;
    QStringList flagDescriptions;
};

struct FGDIO {
    bool isInput = true;
    QString name;
    QString type;
    QString description;
};

struct FGDEntity {
    EntityClassType classType = EntityClassType::PointClass;
    QString className;
    QString description;
    QStringList bases;
    QString studioModel;
    QString iconSprite;
    QString color;
    QString size;
    bool halfGridSnap = false;
    QString sphere;
    QString line;
    QString vecline;
    QString axis;
    QString origin;
    QString wirebox;
    QString lightprop;
    QString studioprop;
    bool decal = false;
    bool overlay = false;
    bool sprite = false;
    bool sweptplayerhull = false;
    bool instance_ = false;
    bool animator = false;
    bool keyframe_ = false;
    bool worldtext = false;
    QList<FGDKeyvalue> keyvalues;
    QList<FGDIO> ios;
};

struct FGDAutoVisGroupChild {
    QString name;
    QStringList entities;
    QList<FGDAutoVisGroupChild> children;
};

struct FGDAutoVisGroup {
    QString parent;
    QList<FGDAutoVisGroupChild> children;
};

struct FGDProject {
    QString headerComment;
    QStringList includes;
    int mapsizeMin = -16384;
    int mapsizeMax = 16384;
    bool useMapsSize = false;
    int fgdVersion = 0;
    QList<FGDEntity> entities;
    QList<FGDAutoVisGroup> autoVisGroups;
    QStringList materialExclusions;
};
