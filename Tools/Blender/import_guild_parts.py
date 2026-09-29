# guild_roof_parts.py·guild_window_wall.py가 만든 FBX를 임포트하고 Hearthvale 재질을 슬롯 이름대로 붙인다.
#   Intermediate/GuildRoof → /Game/Map/Building/Meshes/Roof, Intermediate/GuildWall → /Game/Map/Building/Meshes/Walls
# 에디터를 닫은 상태에서:
#   UnrealEditor-Cmd.exe Asteria.uproject -run=pythonscript -script=Tools/Blender/import_guild_parts.py
import os, unreal

SOURCES = [("GuildRoof", "/Game/Map/Building/Meshes/Roof"), ("GuildWall", "/Game/Map/Building/Meshes/Walls")]
MATS = "/Game/Hearthvale/Material/Instances/"

# UE 5.8 기본 FBX 임포터(Interchange)는 이 경로에서 UCX_ 충돌 메시를 무시한다 → 기존 FBX 임포터로 가져온다.
unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX false")

tasks = []
for folder, dest in SOURCES:
    src = os.path.join(unreal.Paths.project_intermediate_dir(), folder)
    for f in sorted(os.listdir(src) if os.path.isdir(src) else []):
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
        # 다시 가져올 때 FBX에 UCX가 없으면 예전 충돌이 그대로 남아 쌓인다 → 기존 충돌을 먼저 지운다.
        old = unreal.load_asset("%s/%s" % (dest, f[:-4])) if unreal.EditorAssetLibrary.does_asset_exist("%s/%s" % (dest, f[:-4])) else None
        if isinstance(old, unreal.StaticMesh):
            unreal.EditorStaticMeshLibrary.remove_collisions(old)  # 명령줄 실행에선 에디터 서브시스템이 없다
        t = unreal.AssetImportTask()
        t.filename = os.path.join(src, f)
        t.destination_path = dest
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
        unreal.log_warning("GUILDROOF %s mats=%s" % (
            path, [m.material_interface.get_name() if m.material_interface else None for m in sm.static_materials]))
