#pragma once
#include "FGDData.h"
#include <QString>

class FGDImporter {
public:
    static FGDProject importFromFile(const QString &path, QString &errorOut);
private:
    struct Token {
        enum Type { Word, String, Number, LBracket, RBracket, LParen, RParen, Equals, Comma, Colon, Plus, At, Eof } type;
        QString value;
    };

    QList<Token> m_tokens;
    int m_pos = 0;

    bool tokenize(const QString &src);
    Token peek(int offset = 0);
    Token consume();
    bool atEnd();

    bool parseEntity(FGDEntity &e);
    bool parseClassProps(FGDEntity &e);
    bool parseKeyvalue(FGDKeyvalue &kv);
    bool parseIO(FGDIO &io);
    QString parseDescription();

    FGDProject parse();
};
