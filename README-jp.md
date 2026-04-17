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

* `BUILD_CPP_SAMPLES` : C++ sample application をビルドする（default: `OFF`）
* `BUILD_CTEST` : CTest をビルドする（default: `OFF`。`GoogleTest` が自動取得されます）
* `BUILD_PYTHON_WRAPPER` : Python wrapper module と wheel をビルドする（default: `OFF`）

<br>

### 2.1. C++ Library (`vmdio`) のビルド
プロジェクト設定時は、使用する Visual Studio のバージョンに合わせて適切な toolset を指定してください（例えば Visual Studio 17 2022 Generator を使用する場合は `v143`）。

```
cmake -S . -B build -A x64 -T v143
cmake --build build --config Release
```

<br>

`BUILD_CTEST` を `ON` にした場合、CTest をビルド・実行できます。

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

本ライブラリの public API の範囲では、文字列は `UTF-8` エンコードされた `std::string` として扱われます。ライブラリ利用時は、コードエディタおよびターミナルのエンコーディングを `UTF-8` に設定してください。

一方、VMD ファイル内のキーフレームデータのフィールド等の文字列は固定長の `Shift_JIS` ですが、これらのエンコーディング変換はライブラリ内部で処理されます。書き込みの際には、`Shift_JIS` に変換できない文字を含む場合、または変換後のバイト数が既定のフィールド長を超える場合は例外を送出します。また、読み込みの際も、ライブラリが `Shift_JIS` から `UTF-8` へのデコードに失敗した場合は例外を送出します。


[CMakeLists.txt](/CMakeLists.txt) での以下の設定により、ライブラリおよびプロジェクトのソースコードは `UTF-8` エンコードでコンパイルされるようになっています。

```cmake
# Set UTF-8 encoding
target_compile_options(vmdio PUBLIC /utf-8)
```

<br>

### 4.2. 読み書きの関数におけるバリデーション

VMD ファイルの読み込みの関数 `readVMD()` は、ライブラリのデータ構造で表現可能な値については
VMD ファイル中の値を可能な限りそのまま読み取ります。ただし、ライブラリのデータ構造で表現できないもの（例: 定義外のフラグ値など）や、文字列のデコードに失敗する場合は例外を送出します。

VMD ファイルの書き込みの関数 `writeVMD()` は、書き込みの処理の前に必要なバリデーションを行い、不正な値が検出された場合や、文字列のエンコードに失敗する場合は例外を送出します。

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
Python wrapper module `pyvmdio` は pybind11 で実装されており、`vmdio` ライブラリと同一の識別子を Python wrapper でも使用しているため、API は C++ と Python でほぼ一致しています。

`pyvmdio` は Python wheel としてリリースされており、`pip` でインストールできます。

```
pip install <path to pyvmdio-*.whl>
```

<br>

基本的な使用例は以下の通りです。

```python
import pyvmdio.model_edit as vmdio

vmd_data = vmdio.readVMD("<path to vmd file>")

print(f"Model Name: {vmd_data.modelName}")
```

<br>

## 5. ライセンス

本プロジェクトは `BSD 3-Clause License` のもとで公開されています。

使用しているサードパーティライブラリについては、[LICENSES](/LICENSES)
ディレクトリを参照してください。