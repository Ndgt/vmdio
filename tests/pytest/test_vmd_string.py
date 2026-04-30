import pytest

import pyvmdio
import pyvmdio.exceptions as vmdio_exceptions
from pyvmdio.vmd_string import VMDString


def test_vmd_string_from_utf8():
    value = VMDString.fromUTF8("センター")

    assert value.toUTF8() == "センター"
    assert value.toUTF8ForDisplay() == "センター"
    assert value.toShiftJIS() == "センター".encode("cp932")
    assert value.sizeofShiftJISBytes() == len("センター".encode("cp932"))


def test_vmd_string_from_shift_jis():
    shift_jis_bytes = "まばたき".encode("cp932")

    value = VMDString.fromShiftJIS(shift_jis_bytes)

    assert value.toShiftJIS() == shift_jis_bytes
    assert value.toUTF8() == "まばたき"


def test_vmd_string_from_shift_jis_bytes_alias():
    shift_jis_bytes = "左足ＩＫ".encode("cp932")

    value = VMDString.fromShiftJIS(shift_jis_bytes)

    assert value.toShiftJIS() == shift_jis_bytes
    assert value.toUTF8() == "左足ＩＫ"


def test_vmd_string_empty_and_bool():
    value = VMDString()

    assert value.empty()
    assert not value


def test_vmd_string_str_uses_display_conversion():
    value = VMDString.fromUTF8("センター")

    assert str(value) == "センター"


def test_vmd_string_invalid_shift_jis_display():
    value = VMDString.fromShiftJIS(b"\x82")

    assert value.toUTF8ForDisplay() == "?"

    with pytest.raises(vmdio_exceptions.StringProcessError):
        value.toUTF8()


def test_vmd_string_equality_compares_shift_jis_bytes():
    value1 = VMDString.fromUTF8("センター")
    value2 = VMDString.fromShiftJIS("センター".encode("cp932"))
    value3 = VMDString.fromUTF8("上半身")

    assert value1 == value2
    assert value1 != value3


def test_root_module_exports_vmd_string():
    value = pyvmdio.VMDString.fromUTF8("センター")

    assert value.toUTF8() == "センター"