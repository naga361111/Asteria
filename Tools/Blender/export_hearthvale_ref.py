# guild_window_wall.py가 쓰는 Hearthvale 원본 메시를 FBX로 내보낸다(Intermediate/HearthvaleRef/).
# 에디터를 닫은 상태에서:
#   UnrealEditor-Cmd.exe Asteria.uproject -run=pythonscript -script=Tools/Blender/export_hearthvale_ref.py
import os, unreal

OUT = os.path.join(unreal.Paths.project_intermediate_dir(), "HearthvaleRef")
MESHES = ["/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_C", "/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Window"]

os.makedirs(OUT, exist_ok=True)
for path in MESHES:
    t = unreal.AssetExportTask()
    t.object = unreal.load_asset(path)
    t.filename = os.path.join(OUT, path.split("/")[-1] + ".fbx")
    t.automated = True
    t.replace_identical = True
    t.prompt = False
    t.exporter = unreal.StaticMeshExporterFBX()
    t.options = unreal.FbxExportOption()
    unreal.log_warning("HEARTHVALEREF %s %s" % (path, unreal.Exporter.run_asset_export_task(t)))
