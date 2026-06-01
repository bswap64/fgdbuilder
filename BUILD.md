# Building FGDBuilder

## Requirements

| | Windows | Linux |
|---|---|---|
| Compiler | MinGW-w64 (MSYS2) | GCC |
| Qt | Qt5 or Qt6 (Core + Widgets) | Qt5 or Qt6 (Core + Widgets) |
| Build tools | qmake | qmake |

---

## Windows (MSYS2 / MinGW-w64)

```bash
qmake fgdbuilder.pro
mingw32-make
```

---

## Linux

```bash
qmake fgdbuilder.pro
make
```

---

## Output

| Platform | Binary |
|---|---|
| Windows | `release/fgdbuilder.exe` |
| Linux | `release/fgdbuilder` |
