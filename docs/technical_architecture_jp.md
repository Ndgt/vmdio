## Technical Architecture

### 1. 概要

VMD (Vocaloid Motion Data) ファイルは2種類のデータに分かれており、MMD での import 時にはこれらが厳密に区別されます。MMD の現在の編集モード（"モデル編集" または "カメラ編集"）に応じて、データの種類が一致しない場合はインポートできません。

このため、本ライブラリはこのような MMD でのデータ（フォーマット）の区別を反映したアーキテクチャになっています。

ライブラリには、これら 2 種類の VMD データに対応する2つの主要な namespace と、エンコード変換の関数およびカスタム例外クラスを提供する追加の namespace があります。

| Namespace            | Header file     | Description                 |
| -------------------- | --------------- | --------------------------- |
| `vmdio::camera_edit` | `camera_edit.h` | カメラ編集用 VMD データの構造と関数 |
| `vmdio::model_edit`  | `model_edit.h`  | モデル編集用 VMD データの構造と関数 |
| `vmdio::encoding`    | `encoding.h`    | 文字列エンコード変換関数 |
| `vmdio::exceptions`  | `exceptions.h`  | カスタム例外クラス |

<br>

扱う VMD データの種類に対応したヘッダのみをインクルードして使用します。また、
必要に応じてエンコード変換やカスタム例外処理のための追加のヘッダをインクルードします。

C++ における基本的な使用例は以下の通りです。

```cpp
// モデル編集用 VMD データの読み書き例

#include <vmdio/model_edit.h>
#include <vmdio/exceptions.h>

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

VMD ファイルの読み書きの主要な関数は `readVMD()` と `writeVMD()` です。
これらは `vmdio::camera_edit` と `vmdio::model_edit` の両 namespace に定義されています。

`readVMD()` は VMD ファイルのバイナリデータを読み取り、より扱いやすい `VMDData` 構造体へ変換します。

```cpp
VMDData readVMD(const std::filesystem::path &pFilePath);
```

<br>

`writeVMD()` は `VMDData` を VMD のバイナリ形式へシリアライズします。

```cpp
void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath);
```

<br>

読み書きの過程で、`vmdio::exceptions` で定義されたカスタム例外が送出されることがあります。例えば、ファイル I/O エラー、フォーマット不一致、不正なフィールド値、文字列エンコード変換エラー、読み込む総フレーム数が上限を超える場合などです。

`readVMD()` は、ライブラリのデータ構造で表現可能な値については、VMD ファイル中の値を可能な限りそのまま保持します。一方で、未定義の enum 相当のフラグ値のようにライブラリのデータ構造で表現できない値や、文字列のデコードに失敗する項目については例外を送出します。

`writeVMD()` は、書き込み前に必要なバリデーションを行い、不正な値が検出された場合や、文字列エンコード変換に失敗する項目については、例外を送出します。このバリデーションにおける検証の内容は以下の通りです。

- `vmdio::model_edit`
  - モデル名が空
  - `MotionFrame` の重複（同一 `boneName` かつ同一 `frameNumber`）
  - `MorphFrame` の重複（同一 `morphName` かつ同一 `frameNumber`）
  - `VisibleIKFrame` の重複（同一 `frameNumber`）
  - 空の `boneName` / `morphName` / `ikBoneName`
  - `Visibility` / `IKState` の不正値
  - `MotionFrame::rotation` のノルムがゼロに近い値
  - `ControlPointSet` の範囲外値

- `vmdio::camera_edit`
  - `CameraFrame` / `LightFrame` / `SelfShadowFrame` の重複（同一 `frameNumber`）
  - `ProjectionType` / `SelfShadowMode` の不正値
  - `ControlPointSet` の範囲外値

<br>

`MotionFrame::rotation` については、ノルムが `0` ではないが `1` ではない場合、シリアライズ時にライブラリ内部で正規化されます。一方、`MorphFrame::value`、`CameraFrame::viewingAngle`、`LightFrame::color`、`SelfShadowFrame::shadowRange` については、MMD の UI 上で一般的に用いられるスライダー範囲に基づくバリデーションは行っていません。これは、MMD ではそうした範囲外の値であっても、直接数値入力された場合には VMD データとして読み書きできるためです。

<br>

### 3. VMD データ構造

`VMDData` は VMD ファイル全体のデータを保持するコンテナの役割を果たす構造体で、各種キーフレームデータの構造体の配列を保持します。

```cpp
// vmdio::model_edit::VMDData 構造体の定義

struct VMDData
{
    std::string modelName;
    std::vector<MotionFrame> motionFrames;
    std::vector<MorphFrame> morphFrames;
    std::vector<VisibleIKFrame> visibleIKFrames;
};
```

<br>

この構造体は両 namespace に存在しますが、camera edit と model edit でフィールドの内容は異なります。なお、`VMDData` は VMD ファイル内の全てのデータを表現しているわけではありません。ヘッダや（カメラ編集のデータの場合の）モデル名などはライブラリ内部で処理するため、ライブラリのユーザーが直接アクセスすることはありません。

<br>

### 4. キーフレームデータ構造

本ライブラリで定義されているキーフレームデータの構造体は以下の通りです。

| Namespace            | Frame Data Structures                          |
| -------------------- | ---------------------------------------------- |
| `vmdio::camera_edit` | `CameraFrame`, `LightFrame`, `SelfShadowFrame` |
| `vmdio::model_edit`  | `MotionFrame`, `MorphFrame`, `VisibleIKFrame`  |

<br>

各構造体は、それぞれのキーフレーム内のデータを表すフィールドを持ちます。

```cpp
// vmdio::model_edit::MotionFrame 構造体の定義
struct MotionFrame
{
    std::string boneName;
    uint32_t frameNumber = 0;
    Position position;
    Quaternion rotation;
    MotionInterpolation interpolation;
};

// vmdio::camera_edit::CameraFrame 構造体の定義
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

<br>全てのキーフレームデータ構造体について、文字列フィールド以外のフィールドにはデフォルト値が設定されています。これらのデフォルト値は全て、MMD の UI における初期値と一致するように設定されています。また、文字列フィールドは空文字で初期化されます。

<br>

### 5. 値のデータ型と構造体

定義されている主なデータ型・構造体は以下の通りです。

| Namespace | Value Types / Structures　|
| --------- |-------------------------- |
| `vmdio::camera_edit` | `ProjectionType`, `SelfShadowMode`, `Position`, `CameraRotation`, `ControlPointSet`, `CameraInterpolation`, `Color` |
| `vmdio::model_edit`  | `IKState`, `Visibility`, `Position`, `Quaternion`, `ControlPointSet`, `MotionInterpolation` |

<br>

これらは、VMD ファイル内のバイナリ表現に比べてより扱いやすいように設計されています。

- フラグ値を enum class で定義

    ```cpp
    enum class IKState : uint8_t
    {
        OFF = 0, 
        ON = 1   
    };
    ```

- 数値配列の代わりに意味のある構造体を定義

    ```cpp
    struct Position
    {
        float x;
        float y;
        float z;
    };
    ```

- 補間データを簡略化して表現

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

<br>

`Position` や `ControlPointSet` は両 namespace に同じ定義がありますが、設計上の分離を維持するために別々に定義されています。

文字列フィールドについて、本ライブラリの公開 API では、これらは `UTF-8` エンコードされた `std::string` として扱われます。一方、VMD ファイル内の文字列フィールドは固定長の `Shift_JIS` フィールドであり、これらのエンコーディング変換はライブラリ内部で行われます。

書き込み時には、ライブラリが `UTF-8` から `Shift_JIS` へ変換したうえで、各フィールドの長さの上限を超える場合や、変換できない文字が含まれる場合は例外を送出します。この時の上限は以下の通りです。

| Field | Byte Length Limit (after Shift_JIS conversion) |
| ----- | ---------------------------------------------- |
| `vmdio::model_edit::VMDData::modelName` | 20 bytes |
| `vmdio::model_edit::MotionFrame::boneName` | 15 bytes |
| `vmdio::model_edit::MorphFrame::morphName` | 15 bytes |
| `vmdio::model_edit::IKData::ikBoneName` | 20 bytes |

<br>また、読み込み時も、ライブラリが `Shift_JIS` から `UTF-8` へ変換し、デコードに失敗した場合は例外を送出します。

<br>

### 6. エンコード変換

`encoding.h` を include することで、`vmdio::encoding` namespace の関数を利用できます。

```cpp
std::string utf8ToShiftJIS(std::string_view pString);
std::string shiftJISToUTF8(std::string_view pString);
```

<br>

これらは本来、読み書きの際の内部処理のために設計されましたが、ユーザーが必要に応じて独自の文字列エンコード変換の目的で使用することもできます。

<br>

### 7. カスタム例外

`exceptions.h` では以下の例外クラスが定義されています。

| Exception Class           | Brief Description |
| ------------------------- | ----------------- |
| `VMDIOError`              | 全ての例外の基底クラス |
| `FileSystemError`         | ファイル I/O 関連のエラー |
| `FrameConflictError`      | フレームの競合がある場合のエラー  |
| `FrameOverflowError`      | フレーム数が上限を超えた場合のエラー |
| `IncompatibleFormatError` | フォーマットの不一致がある場合のエラー |
| `InvalidFieldValueError`  | 不正なフィールド値が存在する場合のエラー |
| `StringProcessError`      | エンコード変換時のエラー |

<br>

これらの例外は、`readVMD()` や `writeVMD()` 実行時にエラーが発生した場合に送出されます。