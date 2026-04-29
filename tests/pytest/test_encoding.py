import pyvmdio.encoding as encoding


def test_utf8_to_shift_jis_returns_bytes():
    shift_jis_bytes = encoding.utf8ToShiftJIS("センター")

    assert isinstance(shift_jis_bytes, bytes)
    assert shift_jis_bytes == "センター".encode("cp932")


def test_shift_jis_to_utf8_accepts_bytes():
    shift_jis_bytes = "センター".encode("cp932")

    utf8_string = encoding.shiftJISToUTF8(shift_jis_bytes)

    assert utf8_string == "センター"