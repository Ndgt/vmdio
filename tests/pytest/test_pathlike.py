import pyvmdio.model_edit as vmdio


def test_readvmd_accepts_pathlib_path(test_motion_data_dir):
    src = test_motion_data_dir / "motion_morph_visibleik.vmd"
    data = vmdio.readVMD(src)
    assert data.modelName


def test_writevmd_accepts_pathlib_path(temp_vmd_path):
    data = vmdio.VMDData()
    data.modelName = "vmdio-test"
    vmdio.writeVMD(data, temp_vmd_path)
    assert temp_vmd_path.exists()