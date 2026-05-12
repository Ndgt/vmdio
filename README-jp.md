[README - English](./README.md)

# vmdio

`vmdio` は、MMD (MikuMikuDance) で使用される VMD (Vocaloid Motion Data) ファイルを読み書きするための、シンプルな C++ インターフェースを提供するライブラリです。このライブラリは、ラッパーモジュール `pyvmdio` として Python からも使用できます。

<br>

## 1. ビルド要件

- C++ Library (`vmdio`)

  - C++17 以上をサポートする MSVC を備えた 64-bit Windows 環境
  - CMake 3.25 以上

- Python Wrapper module および wheel (`pyvmdio`)

  - 上記 C++ library の build requirements
  - `pybind11`
  - Python 3.11 以上（`build` および `pybind11-stubgen` module がインストール済み）

- C++ リファレンスドキュメント（任意）
    - [Doxygen](https://www.doxygen.nl/)
    - [Graphviz](https://www.graphviz.org/)

- CTest tests（任意）

    - [GoogleTest](https://github.com/google/googletest)

<br>

## 2. ライブラリのビルドとインストール

ソースコードを取得し、プロジェクトディレクトリへ移動します。

```
git clone https://github.com/Ndgt/vmdio.git
cd vmdio
```

<br>

プロジェクト設定時に使用可能な CMake Option は以下の通りです。詳細は [CMakeLists.txt](/CMakeLists.txt) を参照してください。

- `BUILD_CPP_SAMPLES` : C++ sample application をビルドする（default: `OFF`）

- `BUILD_CTEST` : CTest をビルドする（default: `OFF`。`GoogleTest` が自動取得されます）

- `BUILD_PYTHON_WRAPPER` : Python wrapper module と wheel をビルドする（default: `OFF`）
- `BUILD_WITH_STATIC_CRT` : ランタイムライブラリのオプションとして `/MT` および `/MTd` を使用する（default: `OFF`。`BUILD_PYTHON_WRAPPER` と併用不可）

<br>

### 2.1. C++ Library (`vmdio`) のビルド
プロジェクト設定時は、使用する Visual Studio のバージョンに合わせて適切な toolset を指定してください（例えば Visual Studio 17 2022 Generator を使用する場合は `v143`）。

```
cmake -S . -B build -A x64 -T v143
cmake --build build --config Release
```

<br>デフォルトでは、`/MD` および `/MDd` を使用してビルドされます。`/MT` および `/MTd` を使用してビルドするには、`BUILD_WITH_STATIC_CRT` オプションを `ON` に設定してください。

```
cmake -S . -B build -A x64 -T v143 -DBUILD_WITH_STATIC_CRT=ON
```

<br>`BUILD_CTEST` を `ON` にした場合、CTest をビルド・実行できます。

```
ctest --test-dir build/tests/ctest -C Release -V
```

<br>

build 後、指定したパスへインストール可能です。

```
cmake --install build --prefix <install_path> --config Release --component vmdio
```

<br>

`<install_path>` には以下のディレクトリが作成されます。

- `include` : ヘッダファイル
- `lib` : コンパイル済みライブラリと `find_package()` 用の CMake 設定ファイル

<br>

### 2.2. `pyvmdio` Python Wrapper と Wheel のビルド

Python wrapper のビルド要件のモジュールをインストールします。

```
pip install build pybind11-stubgen
```

<br>

CMake 設定時に `BUILD_PYTHON_WRAPPER` を `ON` にします。
`pybind11` が見つからない場合、CMake の `FetchContent()` 関数により自動的にダウンロードされます。

```
cmake -S . -B build -A x64 -T v143 -DBUILD_PYTHON_WRAPPER=ON
```

<br>

ビルドを実行します。`pybind11-stubgen` および Python wheel のビルドも同時に実行されます。

```
cmake --build build --config Release
```

<br>

Python wrapper module の pytest を実行できます。

```
pip install pytest
pip install ./build/wheel/dist/pyvmdio-<version>-<tags>.whl
pytest -s ./tests/pytest
```

<br>

## 3. ドキュメント
ライブラリ設計やデータ構造の詳細は [Technical Architecture](/docs/technical_architecture_jp.md) を参照してください。

また、この Technical Architecture ドキュメントを含む、C++ リファレンスドキュメントは `Doxygen` を用いて生成できます。

```
doxygen ./docs/Doxyfile
```

<br>生成された `./docs/build/html/index.html` を開いて参照してください。


<br>

## 4. ライブラリの使用

`vmdio` library および `pyvmdio` module の使用例は以下を参照してください。

- [samples/cpp](./samples/cpp)
- [samples/python](./samples/python)


<br>

### 4.1. 文字列の扱いに関する注意

本ライブラリの public API における VMD 文字列フィールドは `vmdio::VMDString` で表現されます。

`VMDString` は VMD 文字列フィールドのための値型です。内部表現として、VMD ファイルで使用される `Shift_JIS` バイト列を保持します。これにより、VMD ファイルから読み込んだ文字列フィールドのバイト列を可能な限り保持できます。これには、厳密には `UTF-8` にデコードできないバイト列も含まれます。

C++ で通常の表示用文字列から `VMDString` を作る場合は、`UTF-8` 文字列から構築します。

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_string.h>

vmdio::model_edit::VMDData lData;
lData.modelName = vmdio::VMDString::fromUTF8(u8"初音ミク");
```

<br>`model_edit.h` および `camera_edit.h` は、public なデータ構造に `VMDString` フィールドを含むため、内部で `vmd_string.h` を include しています。ただし、ユーザーコード内で `VMDString` の構築や変換を直接行う場合は、依存関係を明示するために `vmd_string.h` を明示的に include することを推奨します。

すでに `Shift_JIS` エンコードされたバイト列を持っている場合は、`fromShiftJIS()` または `fromShiftJISBytes()` を使用します。

```cpp
vmdio::VMDString lName = vmdio::VMDString::fromShiftJIS(lShiftJISBytes);
```

<br>表示用の文字列を取得する場合は `toUTF8ForDisplay()` を使用します。この表示用変換では、不正または不完全なバイト列は `?` に置き換えられます。

```cpp
std::string lDisplayName = lName.toUTF8ForDisplay();
```

<br>厳密な変換を行う場合は `toUTF8()` を使用します。保持しているバイト列を `Shift_JIS` としてデコードできない場合、この関数は例外を送出することがあります。

```cpp
std::string lStrictUTF8Name = lName.toUTF8();
```

<br>VMD ファイルを書き込む際、ライブラリはシリアライズの境界で、保持している `Shift_JIS` バイト列の長さを検証します。VMD のフィールド長の上限を超えている場合は例外を送出します。ライブラリは文字列を暗黙的に切り捨てません。

Python wrapper では、VMD 文字列フィールドに `str`、`bytes`、または `VMDString` を代入できます。

```python
import pyvmdio.model_edit as vmdio

data = vmdio.VMDData()
data.modelName = "初音ミク"          # Python str

frame = vmdio.MotionFrame()
frame.boneName = "センター"          # 内部的に VMDString へ変換される
```

<br>Python の `str` を VMD 文字列フィールドへ代入した場合、その時点で `VMDString` に変換されます。文字列を `Shift_JIS` で表現できない場合は、代入時に `pyvmdio.exceptions.StringProcessError` が送出されます。一方、フィールドのバイト長制限は `writeVMD()` がデータをシリアライズする時点で検証されます。

Python の VMD 文字列フィールドへ代入する `bytes` は、UTF-8 エンコード済みバイト列として扱われます。すでに生の `Shift_JIS` バイト列を持っている場合は、`VMDString.fromShiftJIS()` で明示的に `VMDString` を構築してください。

```python
from pyvmdio.vmd_string import VMDString

frame.boneName = "センター".encode("utf-8")
frame.boneName = VMDString.fromShiftJIS("センター".encode("cp932"))
```

<br>Python のエンコード変換 API では、エンコード済みのバイト列は `bytes` として表現されます。

```python
import pyvmdio.encoding as encoding

shift_jis_bytes = encoding.utf8ToShiftJIS("センター")                  # str -> bytes
shift_jis_bytes = encoding.utf8ToShiftJIS("センター".encode("utf-8"))  # UTF-8 bytes -> bytes
text = encoding.shiftJISToUTF8(shift_jis_bytes)                       # bytes -> str
```

<br>[CMakeLists.txt](/CMakeLists.txt) での以下の設定により、ライブラリおよびプロジェクトのソースコードは `UTF-8` エンコードでコンパイルされるようになっています。

```cmake
# Set UTF-8 encoding
target_compile_options(vmdio PUBLIC /utf-8)
```

<br>

### 4.2. 読み書きの関数におけるバリデーション

VMD ファイルの読み込み関数 `readVMD()` は、ライブラリのデータ構造で表現可能な値については、VMD ファイル中の値を可能な限りそのまま保持します。ただし、定義外のフラグ値のようにライブラリのデータ構造で表現できない値に遭遇した場合は例外を送出します。

VMD 文字列フィールドについては、`readVMD()` は最初の NUL バイトより前にある意味のある `Shift_JIS` バイト列を保持します。不完全または不正な `Shift_JIS` バイト列は `VMDString` 内に保持され、`toUTF8ForDisplay()` により、不正な部分を `?` に置き換えた表示用文字列として取得できます。

VMD ファイルの書き込み関数 `writeVMD()` は、書き込み前に必要なバリデーションを行います。不正な値が検出された場合や、保持している `Shift_JIS` 文字列が VMD のフィールド長上限を超える場合は例外を送出します。

`writeVMD()` はボーンの回転を表すクォータニオンのノルムがゼロに近い場合を不正値として例外を送出します。一方、ゼロではないが正規化されていないクォータニオンについては、シリアライズ時にライブラリ内部で正規化して書き込みます。

<br>

### 4.3. CMake での利用

`vmdio` は CMake package としてリリースされています。依存関係として利用する場合、`FetchContent()` の使用が便利です。

```cmake
find_package(vmdio QUIET)

if(NOT vmdio_FOUND)
    message(STATUS "vmdio not found, fetching it...")

    include(FetchContent)

    FetchContent_Declare(
        vmdio
        GIT_REPOSITORY https://github.com/Ndgt/vmdio.git
        GIT_TAG        <commit hash of release version>
    )

    FetchContent_MakeAvailable(vmdio)
endif()
```

<br>

リリースパッケージをダウンロードしてインストールした場合は、そのインストールパスを `find_package()` の `PATHS` オプションに指定します。

```cmake
find_package(vmdio REQUIRED PATHS <vmdio_install_path>)
```

<br>

その後、プロジェクトの target に `vmdio` をリンクします。

```cmake
target_link_libraries(${PROJECT_NAME} PRIVATE vmdio)
```

<br>

### 4.4. Python での利用

Python wrapper module `pyvmdio` は pybind11 で実装されています。wrapper は C++ API の構造を可能な限り反映しつつ、エンコード済みバイト列については必要に応じて Python の `bytes` 型へ対応付けます。

`pyvmdio` は Python wheel としてリリースされており、`pip` でインストールできます。

```
pip install <path to pyvmdio-*.whl>
```

<br>基本的な使用例は以下の通りです。

```python
import pyvmdio.model_edit as vmdio

vmd_data = vmdio.readVMD("<path to vmd file>")

print(f"Model Name: {vmd_data.modelName.toUTF8ForDisplay()}")
```

<br>通常のテキストを扱う場合、VMD 文字列フィールドには Python の `str` を代入できます。

```python
data = vmdio.VMDData()
data.modelName = "初音ミク"

frame = vmdio.MotionFrame()
frame.boneName = "センター"
```

<br>Python の VMD 文字列フィールドへ代入する `bytes` は、UTF-8 エンコード済みバイト列として扱われます。生の `Shift_JIS` バイト列を扱いたい場合は、明示的に `VMDString` を構築してください。

```python
from pyvmdio.vmd_string import VMDString

frame.boneName = "センター".encode("utf-8")
frame.boneName = VMDString.fromShiftJIS("センター".encode("cp932"))

print(frame.boneName.toShiftJIS())
```

<br>Python のエンコード変換関数では、Unicode テキストには `str`、エンコード済みのバイト列には `bytes` を使用します。

```python
import pyvmdio.encoding as encoding

shift_jis_bytes = encoding.utf8ToShiftJIS("センター")
text = encoding.shiftJISToUTF8(shift_jis_bytes)
```

<br>**注意**: Python の frame list クラスは内部的に C++ の `std::vector` に対応しています。frame list を変更した後は、必要な要素を再取得してください。

<br>

## 5. ライセンス

本プロジェクトは `BSD 3-Clause License` のもとで公開されています。

使用しているサードパーティライブラリについては、[LICENSES](/LICENSES)
ディレクトリを参照してください。