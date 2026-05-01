## Technical Architecture

### 1. 概要

VMD (Vocaloid Motion Data) ファイルは2種類のデータに分かれており、MMD での import 時にはこれらが厳密に区別されます。モデル編集用 VMD ファイルはカメラ編集モードでは読み込めず、カメラ編集用 VMD ファイルはモデル編集モードでは読み込めません。

本ライブラリは、この厳密なフォーマットの分離を public API に反映するように設計されています。モデル編集用 VMD ファイルとカメラ編集用 VMD ファイルは、別々の namespace と別々の public header で扱います。

| Header file | Main API | Use when |
| --- | --- | --- |
| `model_edit.h` | `vmdio::model_edit` | モデル編集用 VMD ファイルを読み書きする場合 |
| `camera_edit.h` | `vmdio::camera_edit` | カメラ編集用 VMD ファイルを読み書きする場合 |
| `vmd_string.h` | `vmdio::VMDString` | VMD 文字列フィールド値を直接構築・変換・確認する場合 |
| `vmd_encoding.h` | `vmdio::encoding` | `UTF-8` / `Shift_JIS` 変換関数を直接使用する場合 |
| `vmd_exceptions.h` | `vmdio::exceptions` | ライブラリ固有の例外クラスを捕捉する場合 |

<br>通常は、扱いたい VMD ファイルの種類に対応するヘッダのみを include すれば使用できます。

```cpp
#include <vmdio/model_edit.h>
```

または、

```cpp
#include <vmdio/camera_edit.h>
```

<br>`vmdio::VMDString` の構築や変換をユーザーコード内で直接行う場合は、依存関係を明示するために `vmd_string.h` を明示的に include してください。

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_string.h>
```

<br>C++ における基本的な使用例は以下の通りです。

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_exceptions.h>

#include <exception>
#include <iostream>

namespace vmd = vmdio::model_edit;
namespace vmd_except = vmdio::exceptions;

int main()
{
    try
    {
        vmd::VMDData lVmdData = vmd::readVMD("path_to_vmd_file.vmd");

        /* キーフレームデータに対する処理 */

        vmd::writeVMD(lVmdData, "path_to_output_vmd_file.vmd");
    }
    catch (const vmd_except::VMDIOError &e)
    {
        std::cerr << "vmdio library threw an error: " << e.what() << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
```

<br>

### 2. VMD ファイルの読み書きの関数

VMD ファイルの読み書きの主要な関数は `readVMD()` と `writeVMD()` です。これらは `vmdio::camera_edit` と `vmdio::model_edit` の両 namespace に定義されています。

`readVMD()` は VMD ファイルのバイナリデータを読み取り、より扱いやすい `VMDData` 構造体へ変換します。

```cpp
VMDData readVMD(const std::filesystem::path &pFilePath);
```

`writeVMD()` は `VMDData` を VMD のバイナリ形式へシリアライズします。

```cpp
void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath);
```

<br>読み書きの過程で、`vmdio::exceptions` で定義されたカスタム例外が送出されることがあります。例えば、ファイル I/O エラー、フォーマット不一致、不正なフィールド値、文字列フィールドのシリアライズ失敗、読み込む総フレーム数が上限を超える場合などです。

`readVMD()` は、ライブラリのデータ構造で表現可能な値については、VMD ファイル中の値を可能な限りそのまま保持します。一方で、未定義の enum 相当のフラグ値のようにライブラリのデータ構造で表現できない値については例外を送出します。

文字列フィールドについては、`readVMD()` は各固定長 VMD 文字列フィールド内の最初の NUL バイトより前にある、意味のある `Shift_JIS` バイト列を保持します。不完全または不正な `Shift_JIS` バイト列は `vmdio::VMDString` 内に保持され、`toUTF8ForDisplay()` により表示用文字列として取得できます。

`writeVMD()` は、シリアライズ前にデータを検証し、不正な値が検出された場合に例外を送出します。文字列フィールドのバイト長制限は、このシリアライズの境界で検証されます。このバリデーションにおける検証内容は以下の通りです。

- `vmdio::model_edit`
  - 総フレーム数がライブラリの上限を超える場合
  - モデル名が空
  - `MotionFrame` の重複（同一 `boneName` かつ同一 `frameNumber`）
  - `MorphFrame` の重複（同一 `morphName` かつ同一 `frameNumber`）
  - `VisibleIKFrame` の重複（同一 `frameNumber`）
  - `MotionFrame` / `MorphFrame` / `IKData` における空の名前フィールド
  - `Visibility` / `IKState` の不正値
  - `MotionFrame::rotation` のノルムがゼロに近い値
  - `ControlPointSet` の範囲外値
  - 保持している `Shift_JIS` バイト長が VMD のフィールドサイズを超える文字列フィールド

- `vmdio::camera_edit`
  - 総フレーム数がライブラリの上限を超える場合
  - `CameraFrame` / `LightFrame` / `SelfShadowFrame` の重複（同一 `frameNumber`）
  - `ProjectionType` / `SelfShadowMode` の不正値
  - `ControlPointSet` の範囲外値

<br>`MotionFrame::rotation` については、ノルムが `0` ではないが `1` ではない場合、シリアライズ時にライブラリ内部で正規化されます。一方、`MorphFrame::value`、`CameraFrame::viewingAngle`、`LightFrame::color`、`SelfShadowFrame::shadowRange` については、MMD の UI 上で一般的に用いられるスライダー範囲に基づくバリデーションは行っていません。これは、MMD ではそうした範囲外の値であっても、直接数値入力された場合には VMD データとして読み書きできるためです。

<br>

### 3. VMD データ構造

`VMDData` は VMD ファイル全体のデータを保持するコンテナの役割を果たす構造体で、各種キーフレームデータ構造体の配列を保持します。

モデル編集用 VMD ファイルでは、構造体は以下のように定義されています。

```cpp
// vmdio::model_edit::VMDData

struct VMDData
{
    vmdio::VMDString modelName;
    std::vector<MotionFrame> motionFrames;
    std::vector<MorphFrame> morphFrames;
    std::vector<VisibleIKFrame> visibleIKFrames;
};
```

<br>カメラ編集用 VMD ファイルでは、構造体は以下のように定義されています。

```cpp
// vmdio::camera_edit::VMDData

struct VMDData
{
    std::vector<CameraFrame> cameraFrames;
    std::vector<LightFrame> lightFrames;
    std::vector<SelfShadowFrame> selfShadowFrames;
};
```

<br>`VMDData` 構造体は VMD ファイル内の全ての raw field を表現しているわけではありません。ファイルヘッダはライブラリ内部で処理されます。また、カメラ編集用 VMD ファイルのモデル名フィールドも、モデル編集用 VMD ファイルとカメラ編集用 VMD ファイルを区別するための固定のモデル名マーカーとしてライブラリ内部で処理されます。

<br>

### 4. キーフレームデータ構造

本ライブラリで定義されているキーフレームデータ構造体は以下の通りです。

| Namespace | Defined frame data structures |
| --- | --- |
| `vmdio::camera_edit` | `CameraFrame`, `LightFrame`, `SelfShadowFrame` |
| `vmdio::model_edit` | `MotionFrame`, `MorphFrame`, `VisibleIKFrame` |

<br>各構造体は、それぞれのキーフレーム内のデータを表すフィールドを持ちます。

モデル編集用フレームの例:

```cpp
// vmdio::model_edit::MotionFrame

struct MotionFrame
{
    vmdio::VMDString boneName;
    uint32_t frameNumber = 0;
    Position position;
    Quaternion rotation;
    MotionInterpolation interpolation;
};
```

カメラ編集用フレームの例:

```cpp
// vmdio::camera_edit::CameraFrame

struct CameraFrame
{
    uint32_t frameNumber = 0;
    float distance = 45.00f;
    Position position;
    CameraRotation rotation;
    CameraInterpolation interpolation;
    int32_t viewingAngle = 30;
    ProjectionType projectionType = ProjectionType::Perspective;
};
```

<br>全てのキーフレームデータ構造体について、各フィールドにはデフォルト値が設定されています。文字列フィールドは空の `vmdio::VMDString` として初期化されます。それ以外のデフォルト値は、該当するものについては MMD の UI における通常の初期値と一致するように設定されています。

<br>

### 5. 値のデータ型と構造体

定義されている主な値型・構造体は以下の通りです。

| Namespace / scope | Defined value types and structures |
| --- | --- |
| `vmdio::camera_edit` | `ProjectionType`, `SelfShadowMode`, `camera_edit::Position`, `CameraRotation`, `camera_edit::ControlPointSet`, `CameraInterpolation`, `Color` |
| `vmdio::model_edit` | `IKState`, `Visibility`, `model_edit::Position`, `Quaternion`, `model_edit::ControlPointSet`, `MotionInterpolation` |
| `vmdio` | `VMDString` |

<br>これらは、VMD ファイル内のバイナリ表現に比べてより扱いやすいように設計されています。

例えば、フラグ値は raw byte ではなく enum class で表現されます。

```cpp
enum class IKState : uint8_t
{
    OFF = 0,
    ON = 1
};
```

数値ベクトルのような値は、raw な float 配列ではなく、意味のある構造体として表現されます。

```cpp
struct Position
{
    float x;
    float y;
    float z;
};
```

補間データは raw な補間バイト列ではなく、制御点の構造体として表現されます。

```cpp
struct ControlPointSet
{
    uint32_t x1 = 20;
    uint32_t y1 = 20;
    uint32_t x2 = 107;
    uint32_t y2 = 107;
};

struct MotionInterpolation
{
    ControlPointSet xPos;
    ControlPointSet yPos;
    ControlPointSet zPos;
    ControlPointSet rot;
};
```

<br>`Position` や `ControlPointSet` は `vmdio::model_edit` と `vmdio::camera_edit` の両方に同じ定義がありますが、意図的に別々に定義されています。これにより、モデル編集用データとカメラ編集用データの設計上の分離を維持し、ユーザーが扱いたい VMD ファイルの種類に応じたヘッダのみを include できるようにしています。

文字列フィールドは `vmdio::VMDString` で表現されます。詳細は次のセクションを参照してください。

<br>

### 6. VMDString

`vmdio::VMDString` は、VMD 文字列フィールドで使用される public な値型です。`vmd_string.h` で定義されています。

VMD ファイルでは、文字列フィールドは固定長の `Shift_JIS` バイトフィールドとして保存されます。`VMDString` は、これらのフィールドに含まれる意味のある文字列バイト列を表し、内部表現として `Shift_JIS` バイト列を保持します。

この設計は、中間データモデル上での暗黙的な文字列切り捨てを避けるためのものです。例えば、PMX モデルデータには、対応する VMD 文字列フィールドの上限よりも長い名前が含まれることがあります。ライブラリは、そうした名前が VMD データ構造に代入された時点で暗黙的に切り捨てるべきではありません。代わりに、値は `VMDString` として保持され、フィールド長の制限は `writeVMD()` がデータをシリアライズする時点で検証されます。

読み込み時には、固定長 VMD 文字列フィールドは `Shift_JIS` バイト列として読み取られます。最初の NUL バイト以降の padding byte は中間データモデルには保持されませんが、NUL バイトより前の実際の文字列バイト列は可能な限り保持されます。これには、他のツールがマルチバイト文字の途中で文字列フィールドを切り捨てたことにより発生した、不完全または不正な `Shift_JIS` バイト列も含まれます。

書き込み時には、ライブラリはシリアライズの境界で、保持している `Shift_JIS` バイト列の長さを検証します。バイト長がフィールドサイズの上限を超える場合は例外を送出します。ライブラリは文字列フィールドを暗黙的に切り捨てません。

フィールド長の上限は以下の通りです。

| Field | Byte length limit |
| --- | --- |
| `vmdio::model_edit::VMDData::modelName` | 20 bytes |
| `vmdio::model_edit::MotionFrame::boneName` | 15 bytes |
| `vmdio::model_edit::MorphFrame::morphName` | 15 bytes |
| `vmdio::model_edit::IKData::ikBoneName` | 20 bytes |

<br>通常の表示用テキストから `VMDString` を作る場合は、`UTF-8` 文字列から構築します。

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_string.h>

vmdio::model_edit::MotionFrame lFrame;
lFrame.boneName = vmdio::VMDString::fromUTF8(u8"センター");
```

<br>すでに `Shift_JIS` エンコードされたバイト列を持っている場合は、`Shift_JIS` から構築します。

```cpp
std::string lShiftJISBytes = /* Shift_JIS encoded bytes */;
vmdio::VMDString lName = vmdio::VMDString::fromShiftJIS(lShiftJISBytes);
```

<br>バイト列が `std::byte` として表現されている場合は、`fromShiftJISBytes()` を使用します。

```cpp
const std::byte *lBytes = /* Shift_JIS bytes */;
std::size_t lSize = /* byte count */;

vmdio::VMDString lName = vmdio::VMDString::fromShiftJISBytes(lBytes, lSize);
```

<br>表示用の変換には `toUTF8ForDisplay()` を使用します。不正または不完全な `Shift_JIS` バイト列は `?` に置き換えられます。

```cpp
std::string lDisplayName = lName.toUTF8ForDisplay();
```

<br>厳密な変換には `toUTF8()` を使用します。保持しているバイト列を `Shift_JIS` としてデコードできない場合、この関数は例外を送出することがあります。

```cpp
std::string lUTF8Name = lName.toUTF8();
```

<br>保持している `Shift_JIS` バイト列を `std::string` として取得するには、`toShiftJIS()` を使用します。

```cpp
std::string lShiftJISName = lName.toShiftJIS();
```

<br>主な構築関数は以下の通りです。

```cpp
static VMDString fromUTF8(std::string_view pUTF8String);

static VMDString fromShiftJIS(std::string_view pShiftJISBytes);

static VMDString fromShiftJISBytes(const std::byte *pBytes, std::size_t pSize);
```

<br>主なアクセサは以下の通りです。

```cpp
std::string toShiftJIS() const;

std::string toUTF8() const;

std::string toUTF8ForDisplay(bool pStopAtNul = true) const;

std::size_t sizeofShiftJISBytes() const noexcept;

const std::vector<std::byte> &shiftJISBytes() const noexcept;
```

<br>`model_edit.h` のようなヘッダは、public な構造体が `VMDString` フィールドを公開しているため、内部で `vmd_string.h` を include しています。ただし、ユーザーコードで `VMDString` の構築や変換を直接行う場合は、`vmd_string.h` を明示的に include してください。これにより、ユーザーコード上の依存関係が明確になり、推移的 include への依存を避けられます。

<br>

#### Python wrapper における挙動

Python wrapper では、`VMDString` は `pyvmdio.VMDString` および `pyvmdio.vmd_string.VMDString` として公開されます。

Python の VMD 文字列フィールドには、`str`、`bytes`、または `VMDString` を代入できます。

```python
import pyvmdio.model_edit as vmdio
from pyvmdio.vmd_string import VMDString

frame = vmdio.MotionFrame()

frame.boneName = "センター"                                      # Python str
frame.boneName = "センター".encode("utf-8")                     # UTF-8 bytes
frame.boneName = VMDString.fromUTF8("センター")                 # 明示的な VMDString
frame.boneName = VMDString.fromShiftJIS("センター".encode("cp932"))  # 生の Shift_JIS bytes
```

<br>`str` および UTF-8 `bytes` は、代入時に `VMDString` へ変換されます。文字列を `Shift_JIS` で表現できない場合は、代入時に `pyvmdio.exceptions.StringProcessError` が送出されます。

生の `Shift_JIS` バイト列を扱う場合は、`bytes` を文字列フィールドへ直接代入せず、`VMDString.fromShiftJIS()` で明示的に `VMDString` を構築してください。

フィールドのバイト長制限は代入時には検証されません。C++ API の設計と同様に、`writeVMD()` がデータをシリアライズする時点で検証されます。

`VMDString.toShiftJIS()` は Python の `bytes` を返します。一方、`VMDString.toUTF8()` と `VMDString.toUTF8ForDisplay()` は Python の `str` を返します。

```python
name = VMDString.fromUTF8("センター")

shift_jis_bytes = name.toShiftJIS()
display_text = name.toUTF8ForDisplay()
```

<br>Python の frame list クラスは内部的に C++ の `std::vector` に対応しています。`frame = data.motionFrames[0]` のように取得した要素参照を保持したまま、同じ list に `append` / `insert` / `extend` / `remove` / `clear` などの変更操作を行わないでください。list を変更した後は、必要な要素を再取得してください。

<br>

### 7. エンコード変換

`vmd_encoding.h` を include することで、`vmdio::encoding` namespace の関数を利用できます。

```cpp
std::string utf8ToShiftJIS(std::string_view pString);

std::string shiftJISToUTF8(std::string_view pString);
```

<br>これらの関数は、ライブラリ内部で `UTF-8` と `Shift_JIS` の変換に使用されます。ユーザーが必要に応じて直接使用することもできます。

C++ API では、`UTF-8` テキストと `Shift_JIS` エンコード済みバイト列の両方を `std::string` で表現します。そのため、文字列の意味は関数によって異なります。

- `utf8ToShiftJIS()` は `UTF-8` テキストを受け取り、`Shift_JIS` エンコード済みバイト列を `std::string` として返します。
- `shiftJISToUTF8()` は `Shift_JIS` エンコード済みバイト列を `std::string` として受け取り、`UTF-8` テキストを返します。

<br>`VMDString::fromUTF8()` は内部で `UTF-8` から `Shift_JIS` への変換を使用します。`VMDString::toUTF8()` および `VMDString::toUTF8ForDisplay()` は内部で `Shift_JIS` から `UTF-8` への変換を使用します。`toUTF8ForDisplay()` では、不正または不完全なバイト列が `?` に置き換えられます。

Python wrapper では、この区別は Python の `str` と `bytes` によって表現されます。

```python
import pyvmdio.encoding as encoding

shift_jis_bytes = encoding.utf8ToShiftJIS("センター")                  # str -> bytes
shift_jis_bytes = encoding.utf8ToShiftJIS("センター".encode("utf-8"))  # UTF-8 bytes -> bytes
text = encoding.shiftJISToUTF8(shift_jis_bytes)                       # bytes -> str
```

Python の `encoding.utf8ToShiftJIS()` は、Python `str` または UTF-8 エンコード済み `bytes` を受け取り、`Shift_JIS` の `bytes` を返します。`encoding.shiftJISToUTF8()` は `Shift_JIS` の `bytes` を受け取り、Python `str` を返します。

<br>

### 8. カスタム例外

`vmd_exceptions.h` では以下の例外クラスが定義されています。

| Exception class | Brief description |
| --- | --- |
| `VMDIOError` | 全ての例外の基底クラス |
| `FileSystemError` | ファイル I/O 関連のエラー |
| `FrameConflictError` | フレームの競合がある場合のエラー |
| `FrameOverflowError` | フレーム数がライブラリの上限を超えた場合のエラー |
| `IncompatibleFormatError` | フォーマットの不一致がある場合のエラー |
| `InvalidFieldValueError` | 不正なフィールド値が存在する場合のエラー |
| `StringProcessError` | エンコード変換時のエラー |

<br>これらの例外は、`readVMD()`、`writeVMD()`、`VMDString` の変換関数、またはエンコード変換関数の処理中にエラーが発生した場合に送出されることがあります。