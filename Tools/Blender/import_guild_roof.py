# guild_roof_parts.py가 만든 FBX를 /Game/Map/Building/Meshes/Roof 로 임포트하고 Hearthvale 재질을 슬롯 이름대로 붙인다.
# 에디터를 닫은 상태에서:
#   UnrealEditor-Cmd.exe Asteria.uproject -run=pythonscript -script=Tools/Blender/import_guild_roof.py
import os, unreal

SRC = os.path.join(unreal.Paths.project_intermediate_dir(), "GuildRoof")
DEST = "/Game/Map/Building/Meshes/Roof"
MATS = "/Game/Hearthvale/Material/Instances/"

tasks = []
for f in sorted(os.listdir(SRC)):
    if not f.endswith(".fbx"):
        continue
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_materials = False
    ui.import_textures = False
    ui.import_as_skeletal = False
    ui.static_mesh_import_data.combine_meshes = True
    ui.static_mesh_import_data.auto_generate_collision = False
    ui.static_mesh_import_data.generate_lightmap_u_vs = True
    t = unreal.AssetImportTask()
    t.filename = os.path.join(SRC, f)
    t.destination_path = DEST
    t.automated = True
    t.replace_existing = True
    t.save = False
    t.options = ui
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for t in tasks:
    for path in t.imported_object_paths:
        sm = unreal.load_asset(path)
        if not isinstance(sm, unreal.StaticMesh):
            continue
        mats = sm.static_materials
        for i, m in enumerate(mats):
            name = str(m.material_slot_name)
            mi = unreal.load_asset(MATS + name.split(".")[0])
            if mi:
                sm.set_material(i, mi)
        unreal.EditorAssetLibrary.save_loaded_asset(sm)
        hulls = unreal.EditorStaticMeshLibrary.get_convex_collision_count(sm)
        unreal.log_warning("GUILDROOF %s mats=%s hulls=%d" % (
            path, [m.material_interface.get_name() if m.material_interface else None for m in sm.static_materials], hulls))
