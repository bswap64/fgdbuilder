#pragma once
#include "FGDData.h"
#include <QString>

class FGDGenerator {
public:
    static QString generate(const FGDProject &project);
private:
    static QString generateEntity(const FGDEntity &entity);
    static QString generateKeyvalue(const FGDKeyvalue &kv);
    static QString generateIO(const FGDIO &io);
    static QString escapeDescription(const QString &desc);
};
