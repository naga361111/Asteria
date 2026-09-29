# Tavern_C(400) 벽 줄에 끼울 창 벽 SM_GuildWall_Window 생성. 텍스처·재질은 Hearthvale 원본만 쓴다.
#   아래 0~300: SM_Wall_Tavern_D_Window 원본(창·창틀·유리 그대로). 벽면만 Tavern_C와 같은 돌/판자 띠 UV로 다시 입힌다.
#   위 300~400: SM_Wall_Tavern_C 원본의 판자 띠 윗부분·윗단 몰딩을 그대로 붙인다.
#   Tavern_C의 걸레받이·중간 몰딩(240~278)도 붙이고, 중간 몰딩은 창틀 구간만 잘라낸다.
#   D_Window 실내면의 곡선 장식 목재·밑단 목재는 옆 벽과 맞지 않아 뺀다.
# 입력: Intermediate/HearthvaleRef/SM_Wall_Tavern_D_Window.fbx, SM_Wall_Tavern_C.fbx (UE에서 FBX로 내보낸 원본)
# 출력: Intermediate/GuildWall/SM_GuildWall_Window.fbx  (임포트: Tools/Blender/import_guild_roof.py)
# 실행: blender -b --factory-startup --python Tools/Blender/guild_window_wall.py
import bpy, bmesh, os
from mathutils import Vector

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "Intermediate")
SRC = os.path.join(ROOT, "HearthvaleRef")
OUT = os.path.join(ROOT, "GuildWall")
NAME = "SM_GuildWall_Window"

# Blender 좌표(m). 실내면 y≈0(법선 -Y), 바깥면 y≈0.43(법선 +Y).
BAND_Z = 2.48          # Tavern_C 돌 → 판자 띠 경계
SPLICE_Z = 3.0         # 이 위는 Tavern_C 원본을 쓴다
FRAME_X = (0.97, 2.03) # 중간 몰딩을 잘라낼 창틀 구간(창틀 x 0.92~2.08 안쪽이라 잘린 끝이 창틀 뒤로 숨는다)
STONE = "MI_StoneWallBrick_A"
BAND = "MI_Trim_Wood_B_Rough"


def load(fbx):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=os.path.join(SRC, fbx))
    ob = [o for o in bpy.data.objects if o not in before and o.type == 'MESH' and "LOD0" in o.name and not o.name.startswith("UCX")][0]
    bm = bmesh.new(); bm.from_mesh(ob.data); bm.transform(ob.matrix_world)
    return ob, bm


def islands(bm, faces):
    seen, out = set(), []
    for f in faces:
        if f in seen:
            continue
        stack, comp = [f], []
        seen.add(f)
        while stack:
            g = stack.pop(); comp.append(g)
            for e in g.edges:
                for h in e.link_faces:
                    if h not in seen and h.material_index == g.material_index:
                        seen.add(h); stack.append(h)
        out.append(comp)
    return out


def zr(faces):
    zs = [v.co.z for f in faces for v in f.verts]
    return min(zs), max(zs)


def cut(bm, faces, co, no):
    geom = list({e for f in faces for e in f.edges}) + list(faces) + list({v for f in faces for v in f.verts})
    bmesh.ops.bisect_plane(bm, geom=geom, plane_co=co, plane_no=no)


def wall_uv(p, n, band):
    """Tavern_C 실측 UV: U 300cm당 1, 돌 V=0.077+z·0.333, 판자 띠 V=0.502+(z-2.48)·0.345."""
    v_of_z = (0.502 + (p.z - BAND_Z) * (0.494 / 1.43)) if band else (0.077 + p.z * (0.825 / 2.48))
    a = max(range(3), key=lambda i: abs(n[i]))
    if a == 1:
        return (p.x / 3.0 if n.y < 0 else 1.0 - p.x / 3.0, v_of_z)
    if a == 0:
        return (0.433 + p.y * (0.134 / 0.4), v_of_z)
    return (p.x / 3.0, 0.433 + p.y * (0.134 / 0.4))


def main():
    os.makedirs(OUT, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)

    # ---- 창 벽(D_Window) ----
    wo, wb = load("SM_Wall_Tavern_D_Window.fbx")
    wmats = [m.name.split(".")[0] for m in wo.data.materials]
    kill = []
    for comp in islands(wb, list(wb.faces)):
        m = wmats[comp[0].material_index]
        lo, hi = zr(comp)
        if m == "MI_PlanksFLoor" or (m == "MI_Trim_Wood_A_Rough" and hi <= 0.15):
            kill += comp                      # 실내 곡선 장식·가로대, 밑단 목재
    bmesh.ops.delete(wb, geom=kill, context='FACES')
    kill = [f for f in wb.faces if f.calc_center_median().z > SPLICE_Z - 0.001 and wmats[f.material_index] in ("MI_Plaster_A", STONE)]
    bmesh.ops.delete(wb, geom=kill, context='FACES')  # 윗면(300)은 Tavern_C 원본이 이어받는다

    # 유리는 한쪽 면만 그려진다. 원본은 실내를 향해 안에서는 발광면에 막히고 밖에서 안이 보인다 → 뒤집어 안에서 밖이 보이게.
    bmesh.ops.reverse_faces(wb, faces=[f for f in wb.faces if wmats[f.material_index] == "MI_Window_Emissive"])

    wall = [f for f in wb.faces if wmats[f.material_index] in ("MI_Plaster_A", STONE)]
    cut(wb, wall, Vector((0, 0, BAND_Z)), Vector((0, 0, 1)))
    if BAND not in wmats:
        wo.data.materials.append(bpy.data.materials.get(BAND) or bpy.data.materials.new(BAND)); wmats.append(BAND)
    i_stone, i_band = wmats.index(STONE), wmats.index(BAND)
    uv = wb.loops.layers.uv[0]
    for f in wb.faces:
        if wmats[f.material_index] not in ("MI_Plaster_A", STONE):
            continue
        c = f.calc_center_median()
        # 판자 띠는 벽 평면(실내 y 0~0.03, 바깥 y 0.43)에 있는 면만. 창 둘레 돌 테두리·창 안쪽 면은 돌로 둔다.
        on_plane = abs(f.normal.y) > 0.9 and (c.y < 0.05 or c.y > 0.42)
        band = c.z > BAND_Z and on_plane
        f.material_index = i_band if band else i_stone
        for l in f.loops:
            l[uv].uv = wall_uv(l.vert.co, f.normal, band)

    # ---- Tavern_C 원본에서 가져올 부분: 걸레받이, 중간 몰딩(창틀 구간 제외), 300 위 전부 ----
    co, cb = load("SM_Wall_Tavern_C.fbx")
    cmats = [m.name.split(".")[0] for m in co.data.materials]  # 같은 재질이 "이름.001"로 들어오므로 기본 이름으로 비교
    cut(cb, [f for f in cb.faces if zr([f])[1] > SPLICE_Z and zr([f])[0] < SPLICE_Z], Vector((0, 0, SPLICE_Z)), Vector((0, 0, 1)))
    molding = [f for f in cb.faces if cmats[f.material_index] == BAND and 2.39 <= zr([f])[0] and zr([f])[1] <= 2.79]
    for x in FRAME_X:
        cut(cb, molding, Vector((x, 0, 0)), Vector((1, 0, 0)))
        molding = [f for f in cb.faces if cmats[f.material_index] == BAND and 2.39 <= zr([f])[0] and zr([f])[1] <= 2.79]
    keep = set()
    for f in cb.faces:
        c = f.calc_center_median(); lo, hi = zr([f])
        if c.z > SPLICE_Z:
            keep.add(f)                                   # 판자 띠 윗부분·윗단 몰딩
        elif cmats[f.material_index] == BAND and hi <= 0.371:
            keep.add(f)                                   # 걸레받이
        elif cmats[f.material_index] == BAND and 2.39 <= lo and hi <= 2.79 and not (FRAME_X[0] < c.x < FRAME_X[1]):
            keep.add(f)                                   # 중간 몰딩(창틀 구간 제외)
    bmesh.ops.delete(cb, geom=[f for f in cb.faces if f not in keep], context='FACES')

    # ---- 합치기 ----
    def to_obj(bm, src, name):
        me = bpy.data.meshes.new(name); bm.to_mesh(me)
        for m in src.data.materials:
            me.materials.append(m)
        ob = bpy.data.objects.new(name, me); bpy.context.scene.collection.objects.link(ob)
        return ob
    a = to_obj(wb, wo, NAME)
    b = to_obj(cb, co, NAME + "_top")
    for o in list(bpy.data.objects):
        if o not in (a, b):
            bpy.data.objects.remove(o)
    bpy.ops.object.select_all(action='DESELECT')
    a.select_set(True); b.select_set(True); bpy.context.view_layer.objects.active = a
    bpy.ops.object.join()
    # 두 FBX에서 같은 재질이 "이름.001"로 중복 들어온다 → 기본 이름 기준으로 한 칸에 모은다.
    first = {}
    for i, m in enumerate(a.data.materials):
        first.setdefault(m.name.split(".")[0], i)
    remap = {i: first[m.name.split(".")[0]] for i, m in enumerate(a.data.materials)}
    for p in a.data.polygons:
        p.material_index = remap[p.material_index]
    for i, m in enumerate(a.data.materials):
        m.name = m.name.split(".")[0] if first[m.name.split(".")[0]] == i else m.name
    # 안 쓰는 재질 칸 제거
    used = {p.material_index for p in a.data.polygons}
    for i in reversed(range(len(a.data.materials))):
        if i not in used:
            a.active_material_index = i
            bpy.ops.object.material_slot_remove()

    # 충돌: 벽 전체를 덮는 상자 하나(창은 닫힌 창이라 막아도 된다).
    hb = bmesh.new()
    vs = [hb.verts.new((x, y, z)) for x in (0.0, 3.0) for y in (-0.06, 0.44) for z in (0.0, 4.0)]
    bmesh.ops.convex_hull(hb, input=vs)
    me = bpy.data.meshes.new("UCX_%s_00" % NAME); hb.to_mesh(me)
    ucx = bpy.data.objects.new(me.name, me); bpy.context.scene.collection.objects.link(ucx)
    objs = [a, ucx]

    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        o.select_set(True)
    bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, NAME + ".fbx"), use_selection=True, apply_unit_scale=False, global_scale=1.0,
                             apply_scale_options="FBX_SCALE_NONE", mesh_smooth_type="FACE", add_leaf_bones=False,
                             bake_anim=False, axis_forward="-Z", axis_up="Y")
    vs = [v.co for v in a.data.vertices]
    print("WINWALL tris", sum(len(p.vertices) - 2 for p in a.data.polygons), "mats", [m.name for m in a.data.materials],
          "x[%.2f,%.2f] y[%.2f,%.2f] z[%.2f,%.2f]" % (min(v.x for v in vs), max(v.x for v in vs), min(v.y for v in vs),
                                                      max(v.y for v in vs), min(v.z for v in vs), max(v.z for v in vs)))


main()
