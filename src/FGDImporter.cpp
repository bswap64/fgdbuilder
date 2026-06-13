#include "FGDImporter.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <functional>

bool FGDImporter::tokenize(const QString &src) {
    int i = 0;
    int len = src.length();
    while (i < len) {
        QChar c = src[i];
        if (c == '/' && i + 1 < len && src[i+1] == '/') {
            while (i < len && src[i] != '\n') ++i;
            continue;
        }
        if (c.isSpace()) { ++i; continue; }
        if (c == '@') { Token t; t.type = Token::At; t.value = "@"; m_tokens.append(t); ++i; continue; }
        if (c == '[') { Token t; t.type = Token::LBracket; t.value = "["; m_tokens.append(t); ++i; continue; }
        if (c == ']') { Token t; t.type = Token::RBracket; t.value = "]"; m_tokens.append(t); ++i; continue; }
        if (c == '(') { Token t; t.type = Token::LParen; t.value = "("; m_tokens.append(t); ++i; continue; }
        if (c == ')') { Token t; t.type = Token::RParen; t.value = ")"; m_tokens.append(t); ++i; continue; }
        if (c == '=') { Token t; t.type = Token::Equals; t.value = "="; m_tokens.append(t); ++i; continue; }
        if (c == ',') { Token t; t.type = Token::Comma; t.value = ","; m_tokens.append(t); ++i; continue; }
        if (c == ':') { Token t; t.type = Token::Colon; t.value = ":"; m_tokens.append(t); ++i; continue; }
        if (c == '+') { Token t; t.type = Token::Plus; t.value = "+"; m_tokens.append(t); ++i; continue; }
        if (c == '"') {
            ++i;
            QString s;
            while (i < len && src[i] != '"') {
                if (src[i] == '\\' && i + 1 < len && src[i+1] == '\\') { s += '\\'; i += 2; }
                else { s += src[i]; ++i; }
            }
            if (i < len) ++i;
            Token t; t.type = Token::String; t.value = s; m_tokens.append(t);
            continue;
        }
        if (c == '-' || c.isDigit()) {
            QString num;
            if (c == '-') { num += c; ++i; }
            while (i < len && (src[i].isDigit() || src[i] == '.')) { num += src[i]; ++i; }
            Token t; t.type = Token::Number; t.value = num; m_tokens.append(t);
            continue;
        }
        if (c.isLetter() || c == '_') {
            QString word;
            while (i < len && (src[i].isLetterOrNumber() || src[i] == '_' || src[i] == '.')) { word += src[i]; ++i; }
            Token t; t.type = Token::Word; t.value = word; m_tokens.append(t);
            continue;
        }
        ++i;
    }
    Token eof; eof.type = Token::Eof; eof.value = "";
    m_tokens.append(eof);
    return true;
}

FGDImporter::Token FGDImporter::peek(int offset) {
    int idx = m_pos + offset;
    if (idx >= m_tokens.size()) return m_tokens.last();
    return m_tokens[idx];
}

FGDImporter::Token FGDImporter::consume() {
    if (m_pos >= m_tokens.size()) return m_tokens.last();
    return m_tokens[m_pos++];
}

bool FGDImporter::atEnd() {
    return peek().type == Token::Eof;
}

QString FGDImporter::parseDescription() {
    QString result;
    while (peek().type == Token::String) {
        result += consume().value;
        if (peek().type == Token::Plus) {
            consume();
        } else {
            break;
        }
    }
    return result;
}

bool FGDImporter::parseClassProps(FGDEntity &e) {
    while (!atEnd() && peek().type != Token::Equals) {
        if (peek().type != Token::Word) { consume(); continue; }
        QString prop = consume().value.toLower();
        if (prop == "base") {
            if (peek().type == Token::LParen) consume();
            while (!atEnd() && peek().type != Token::RParen) {
                if (peek().type == Token::Word) e.bases << consume().value;
                else if (peek().type == Token::Comma) consume();
                else consume();
            }
            if (peek().type == Token::RParen) consume();
        } else if (prop == "color") {
            if (peek().type == Token::LParen) consume();
            QString col;
            while (!atEnd() && peek().type != Token::RParen) {
                col += consume().value + " ";
            }
            if (peek().type == Token::RParen) consume();
            e.color = col.trimmed();
        } else if (prop == "size") {
            if (peek().type == Token::LParen) consume();
            QString sz;
            while (!atEnd() && peek().type != Token::RParen) {
                sz += consume().value + " ";
            }
            if (peek().type == Token::RParen) consume();
            e.size = sz.trimmed();
        } else if (prop == "studio") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::String) e.studioModel = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "studioprop") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::String) e.studioprop = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "iconsprite") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::String) e.iconSprite = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "sphere") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::Word) e.sphere = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "line") {
            if (peek().type == Token::LParen) consume();
            QString val;
            while (!atEnd() && peek().type != Token::RParen) val += consume().value + " ";
            if (peek().type == Token::RParen) consume();
            e.line = val.trimmed();
        } else if (prop == "vecline") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::Word) e.vecline = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "axis") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::Word) e.axis = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "wirebox") {
            if (peek().type == Token::LParen) consume();
            QString val;
            while (!atEnd() && peek().type != Token::RParen) val += consume().value + " ";
            if (peek().type == Token::RParen) consume();
            e.wirebox = val.trimmed();
        } else if (prop == "origin") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::Word || peek().type == Token::String) e.origin = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "lightprop") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::String) e.lightprop = consume().value;
            if (peek().type == Token::RParen) consume();
        } else if (prop == "decal") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.decal = true;
        } else if (prop == "overlay") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.overlay = true;
        } else if (prop == "sprite") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.sprite = true;
        } else if (prop == "sweptplayerhull") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.sweptplayerhull = true;
        } else if (prop == "instance") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.instance_ = true;
        } else if (prop == "animator") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.animator = true;
        } else if (prop == "keyframe") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.keyframe_ = true;
        } else if (prop == "worldtext") {
            if (peek().type == Token::LParen) consume();
            if (peek().type == Token::RParen) consume();
            e.worldtext = true;
        } else if (prop == "halfgridsnap") {
            e.halfGridSnap = true;
        } else {
            if (peek().type == Token::LParen) {
                consume();
                while (!atEnd() && peek().type != Token::RParen) consume();
                if (peek().type == Token::RParen) consume();
            }
        }
    }
    return true;
}

bool FGDImporter::parseKeyvalue(FGDKeyvalue &kv) {
    if (peek().type != Token::Word) return false;
    kv.name = consume().value;
    if (peek().type != Token::LParen) return false;
    consume();
    QString type;
    while (!atEnd() && peek().type != Token::RParen) type += consume().value;
    consume();
    kv.type = type.toLower();

    if (peek().type == Token::Word && (peek().value == "readonly" || peek().value == "report")) {
        QString mod = consume().value;
        if (mod == "readonly") kv.readOnly = true;
        else kv.report = true;
    }
    if (peek().type == Token::Word && (peek().value == "readonly" || peek().value == "report")) {
        QString mod = consume().value;
        if (mod == "readonly") kv.readOnly = true;
        else kv.report = true;
    }

    if (peek().type == Token::Colon) {
        consume();
        if (peek().type == Token::String) kv.displayName = consume().value;
        if (peek().type == Token::Colon) {
            consume();
            if (peek().type == Token::String) kv.defaultValue = consume().value;
            else if (peek().type == Token::Number) kv.defaultValue = consume().value;
            else if (peek().type == Token::Word) kv.defaultValue = consume().value;
            if (peek().type == Token::Colon) {
                consume();
                kv.description = parseDescription();
            }
        }
    }

    if (kv.type == "choices" || kv.type == "flags") {
        if (peek().type == Token::Equals) consume();
        if (peek().type == Token::LBracket) consume();
        while (!atEnd() && peek().type != Token::RBracket) {
            QString bitOrVal;
            if (peek().type == Token::Number) bitOrVal = consume().value;
            else if (peek().type == Token::String) bitOrVal = consume().value;
            else if (peek().type == Token::Word) bitOrVal = consume().value;
            else { consume(); continue; }

            QString label;
            if (peek().type == Token::Colon) {
                consume();
                if (peek().type == Token::String) label = consume().value;
                else if (peek().type == Token::Number) label = consume().value;
                else if (peek().type == Token::Word) label = consume().value;
            }
            if (kv.type == "choices") {
                kv.choiceItems.append({bitOrVal, label});
                QString cdesc;
                if (peek().type == Token::Colon) {
                    consume();
                    cdesc = parseDescription();
                }
                kv.choiceDescriptions.append({bitOrVal, cdesc});
            } else {
                int def = 0;
                if (peek().type == Token::Colon) {
                    consume();
                    if (peek().type == Token::Number || peek().type == Token::Word)
                        def = consume().value.toInt();
                }
                QString fdesc;
                if (peek().type == Token::Colon) {
                    consume();
                    fdesc = parseDescription();
                }
                kv.flagItems.append({bitOrVal, {label, def}});
                kv.flagDescriptions.append(fdesc);
            }
        }
        if (peek().type == Token::RBracket) consume();
    }
    return true;
}

bool FGDImporter::parseIO(FGDIO &io) {
    QString kw = consume().value.toLower();
    io.isInput = (kw == "input");
    if (peek().type != Token::Word) return false;
    io.name = consume().value;
    if (peek().type != Token::LParen) return false;
    consume();
    QString type;
    while (!atEnd() && peek().type != Token::RParen) type += consume().value;
    if (peek().type == Token::RParen) consume();
    io.type = type.toLower();
    if (peek().type == Token::Colon) {
        consume();
        io.description = parseDescription();
    }
    return true;
}

bool FGDImporter::parseEntity(FGDEntity &e) {
    QString classTypeStr = consume().value.toLower();
    if (classTypeStr == "pointclass") e.classType = EntityClassType::PointClass;
    else if (classTypeStr == "solidclass") e.classType = EntityClassType::SolidClass;
    else if (classTypeStr == "npcclass") e.classType = EntityClassType::NPCClass;
    else if (classTypeStr == "keyframeclass") e.classType = EntityClassType::KeyFrameClass;
    else if (classTypeStr == "moveclass") e.classType = EntityClassType::MoveClass;
    else if (classTypeStr == "filterclass") e.classType = EntityClassType::FilterClass;
    else if (classTypeStr == "baseclass") e.classType = EntityClassType::BaseClass;
    else return false;

    parseClassProps(e);

    if (peek().type != Token::Equals) return false;
    consume();
    if (peek().type == Token::Word) e.className = consume().value;
    else return false;

    if (peek().type == Token::Colon) {
        consume();
        e.description = parseDescription();
    }

    if (peek().type != Token::LBracket) return true;
    consume();

    while (!atEnd() && peek().type != Token::RBracket) {
        if (peek().type == Token::Word) {
            QString w = peek().value.toLower();
            if (w == "input" || w == "output") {
                FGDIO io;
                if (parseIO(io)) e.ios.append(io);
            } else {
                FGDKeyvalue kv;
                if (parseKeyvalue(kv)) e.keyvalues.append(kv);
                else consume();
            }
        } else {
            consume();
        }
    }
    if (peek().type == Token::RBracket) consume();
    return true;
}

FGDProject FGDImporter::parse() {
    FGDProject proj;
    while (!atEnd()) {
        if (peek().type == Token::At) {
            consume();
            if (atEnd() || peek().type != Token::Word) continue;
            QString kw = peek().value.toLower();
            if (kw == "version") {
                consume();
                if (peek().type == Token::LParen) consume();
                if (peek().type == Token::Number) proj.fgdVersion = consume().value.toInt();
                if (peek().type == Token::RParen) consume();
            } else if (kw == "include") {
                consume();
                if (peek().type == Token::String) proj.includes << consume().value;
            } else if (kw == "mapsize") {
                consume();
                if (peek().type == Token::LParen) consume();
                if (peek().type == Token::Number) proj.mapsizeMin = consume().value.toInt();
                if (peek().type == Token::Comma) consume();
                if (peek().type == Token::Number) proj.mapsizeMax = consume().value.toInt();
                if (peek().type == Token::RParen) consume();
                proj.useMapsSize = true;
            } else if (kw == "materialexclusion") {
                consume();
                if (peek().type == Token::LBracket) consume();
                while (!atEnd() && peek().type != Token::RBracket) {
                    if (peek().type == Token::String) proj.materialExclusions << consume().value;
                    else consume();
                }
                if (peek().type == Token::RBracket) consume();
            } else if (kw == "autovisgroup") {
                consume();
                if (peek().type == Token::Equals) consume();
                FGDAutoVisGroup avg;
                if (peek().type == Token::String) avg.parent = consume().value;
                if (peek().type == Token::LBracket) consume();
                std::function<FGDAutoVisGroupChild()> parseChild = [&]() -> FGDAutoVisGroupChild {
                    FGDAutoVisGroupChild child;
                    child.name = consume().value;
                    if (peek().type == Token::LBracket) consume();
                    while (!atEnd() && peek().type != Token::RBracket) {
                        if (peek().type == Token::String) {
                            int saved = m_pos;
                            QString str = consume().value;
                            if (peek().type == Token::LBracket) {
                                m_pos = saved;
                                child.children.append(parseChild());
                            } else {
                                child.entities << str;
                            }
                        } else consume();
                    }
                    if (peek().type == Token::RBracket) consume();
                    return child;
                };
                while (!atEnd() && peek().type != Token::RBracket) {
                    if (peek().type == Token::String) avg.children.append(parseChild());
                    else consume();
                }
                if (peek().type == Token::RBracket) consume();
                proj.autoVisGroups.append(avg);
            } else if (kw == "extendclass") {
                consume();
                FGDEntity e;
                e.classType = EntityClassType::PointClass;
                parseClassProps(e);
                if (peek().type == Token::Equals) consume();
                if (peek().type == Token::Word) e.className = consume().value;
                if (peek().type == Token::Colon) { consume(); parseDescription(); }
                if (peek().type == Token::LBracket) {
                    consume();
                    while (!atEnd() && peek().type != Token::RBracket) {
                        if (peek().type == Token::Word) {
                            QString w = peek().value.toLower();
                            if (w == "input" || w == "output") {
                                FGDIO io;
                                if (parseIO(io)) e.ios.append(io);
                            } else {
                                FGDKeyvalue kv;
                                if (parseKeyvalue(kv)) e.keyvalues.append(kv);
                                else consume();
                            }
                        } else consume();
                    }
                    if (peek().type == Token::RBracket) consume();
                }
                for (auto &existing : proj.entities) {
                    if (existing.className == e.className) {
                        for (auto &kv : e.keyvalues) existing.keyvalues.append(kv);
                        for (auto &io : e.ios) existing.ios.append(io);
                        break;
                    }
                }
            } else if (kw == "pointclass" || kw == "solidclass" || kw == "npcclass" ||
                       kw == "keyframeclass" || kw == "moveclass" || kw == "filterclass" || kw == "baseclass") {
                FGDEntity e;
                if (parseEntity(e)) proj.entities.append(e);
            } else {
                consume();
            }
        } else {
            consume();
        }
    }
    return proj;
}

FGDProject FGDImporter::importFromFile(const QString &path, QString &errorOut) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorOut = "Cannot open file: " + path;
        return {};
    }
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    QString src = ts.readAll();
    f.close();

    FGDImporter importer;
    if (!importer.tokenize(src)) {
        errorOut = "Tokenization failed";
        return {};
    }
    return importer.parse();
}
