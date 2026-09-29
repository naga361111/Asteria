# PCG_GuildShell 박공지붕 부품 생성기. 실행:
#   blender -b --factory-startup --python Tools/Blender/guild_roof_parts.py
# 결과 FBX는 Intermediate/GuildRoof/ 에 쓰인다(언리얼 임포트는 Tools/Blender/import_guild_roof.py).
#
# 좌표는 언리얼 기준(cm, X 앞, Y 오른쪽=건물 안쪽, Z 위)으로 적고 ue()로 Blender 좌표(m, Y 반전)로 바꾼다.
# 지붕 기준(Hearthvale 300 격자, Tavern_C 벽 400):
#   경사 45도. 경사판 한 칸 = 수평 150 · 수직 150 · 경사 212.13.
#   경사판 피벗 = 판 밑면의 아래 모서리, X 0~길이, +Y·+Z 방향으로 올라감.
import bpy, bmesh, math, os, random
from mathutils import Vector

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "Intermediate", "GuildRoof")
S45 = math.sqrt(0.5)
RUN = 150.0                      # 경사판 한 칸 수평 길이
SLOPE_LEN = RUN / S45            # 212.13
BOARD = 4.0                      # 지붕 밑판(서까래 위 판재) 두께
SHINGLE_T = 3.0                  # 너와 두께
SHINGLE_LIFT = 4.0               # 너와 아래 끝이 들린 높이(윗줄이 아랫줄 위에 얹힘)
COURSES = 3                      # 경사판 한 칸의 너와 줄 수
SHINGLE_LEN = SLOPE_LEN / COURSES + 20.0
WOOD_UV = 260.0                  # MI_WoodPlanks_Old 반복 길이(sm_beam_wide_300 실측)
PLANK_UV = 300.0                 # MI_PlanksFLoor 반복 길이(SM_PlanksFloor_A 실측)

MAT_SHINGLE = "MI_WoodPlanks_A_Dark"   # Old·C·Old_Light는 붉은 칠·초록 얼룩이 섞여 너와가 얼룩져 보임(2026-09-29 비교)
MAT_UNDER = "MI_PlanksFLoor"
MAT_TRIM = "MI_Trim_Wood_B_Rough"
MAT_PLASTER = "MI_Plaster_A"             # SM_Wall_Tavern_D 회벽


def ue(p):
    return Vector((p[0] * 0.01, -p[1] * 0.01, p[2] * 0.01))


class Builder:
    """언리얼 좌표로 닫힌 상자·기둥을 쌓아 한 메시로 만든다."""

    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.new("UV0")
        self.mats = []
        self.hulls = []

    def mat(self, name):
        if name not in self.mats:
            self.mats.append(name)
        return self.mats.index(name)

    def solid(self, bottom, top, mat, uvfn):
        """bottom/top: 같은 순서의 꼭짓점 링(언리얼 좌표). 옆면·윗면·아랫면을 모두 만든다."""
        vb = [self.bm.verts.new(ue(p)) for p in bottom]
        vt = [self.bm.verts.new(ue(p)) for p in top]
        n = len(bottom)
        rings = [(vb[::-1], bottom[::-1]), (vt, top)]
        faces = [(r, pts) for r, pts in rings]
        for i in range(n):
            j = (i + 1) % n
            faces.append(([vb[i], vb[j], vt[j], vt[i]], [bottom[i], bottom[j], top[j], top[i]]))
        mi = self.mat(mat)
        for verts, pts in faces:
            f = self.bm.faces.new(verts)
            f.material_index = mi
            nrm = (Vector(pts[1]) - Vector(pts[0])).cross(Vector(pts[2]) - Vector(pts[0]))
            for loop, p in zip(f.loops, pts):
                loop[self.uv].uv = uvfn(Vector(p), nrm)

    def ucx(self, bottom, top):
        """단순 충돌(볼록 껍질). UCX_ 접두사 메시로 따로 내보낸다."""
        self.hulls.append((bottom, top))

    def finish(self):
        bmesh.ops.recalc_face_normals(self.bm, faces=self.bm.faces)
        me = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(me)
        for m in self.mats:
            me.materials.append(bpy.data.materials.get(m) or bpy.data.materials.new(m))
        ob = bpy.data.objects.new(self.name, me)
        bpy.context.scene.collection.objects.link(ob)
        objs = [ob]
        for i, (b, t) in enumerate(self.hulls):
            hb = bmesh.new()
            vs = [hb.verts.new(ue(p)) for p in b + t]
            bmesh.ops.convex_hull(hb, input=vs)
            hm = bpy.data.meshes.new("UCX_%s_%02d" % (self.name, i))
            hb.to_mesh(hm)
            ho = bpy.data.objects.new(hm.name, hm)
            bpy.context.scene.collection.objects.link(ho)
            objs.append(ho)
        return objs


def planar_uv(size, axes):
    """면 법선에 가장 가까운 축을 빼고 나머지 두 축으로 평면 투영."""
    def fn(p, n):
        a = max(range(3), key=lambda i: abs(n[i]))
        u, v = [i for i in range(3) if i != a] if axes is None else axes[a]
        return (p[u] / size, p[v] / size)
    return fn


def slope_frame(s, x, h):
    """경사 좌표(s=밑면 따라 올라간 길이, x, h=밑면에서 수직 거리) → 언리얼 좌표."""
    return (x, s * S45 - h * S45, s * S45 + h * S45)


def slope_box(b, x0, x1, s0, s1, h0, h1, mat, uvfn, h1_top=None, h0_top=None):
    """경사면 위 상자. h0_top/h1_top을 주면 아랫면/윗면의 s1쪽 높이가 달라지는 쐐기."""
    h1b = h1 if h1_top is None else h1_top
    h0b = h0 if h0_top is None else h0_top
    bottom = [slope_frame(s0, x0, h0), slope_frame(s0, x1, h0), slope_frame(s1, x1, h0b), slope_frame(s1, x0, h0b)]
    top = [slope_frame(s0, x0, h1), slope_frame(s0, x1, h1), slope_frame(s1, x1, h1b), slope_frame(s1, x0, h1b)]
    b.solid(bottom, top, mat, uvfn)


def shingle_uv(ox, oy):
    # 결이 경사 방향(U)으로 흐르게. s는 밑면 길이로 되돌려 계산.
    def fn(p, n):
        s = (p[1] + p[2]) / (2 * S45)
        return (s / WOOD_UV + ox, p[0] / WOOD_UV + oy)
    return fn


def under_uv(p, n):
    s = (p[1] + p[2]) / (2 * S45)
    return (p[0] / PLANK_UV, s / PLANK_UV)


def build_slope(name, length, slope_len, seed, barge=False, fascia=False):
    """경사판 한 칸(또는 처마 칸). length=용마루 방향 길이, slope_len=밑면 경사 길이."""
    rnd = random.Random(seed)
    b = Builder(name)
    # 밑판: 실내에서 보이는 천장 면.
    slope_box(b, 0, length, 0, slope_len, 0, BOARD, MAT_UNDER, under_uv)
    b.ucx([slope_frame(0, 0, 0), slope_frame(0, length, 0), slope_frame(slope_len, length, 0), slope_frame(slope_len, 0, 0)],
          [slope_frame(0, 0, 12), slope_frame(0, length, 12), slope_frame(slope_len, length, 12), slope_frame(slope_len, 0, 12)])
    # 너와: 줄마다 폭이 다른 조각을 엇갈려 깐다. 아래 끝은 들리고 위 끝은 밑판에 붙는 쐐기.
    courses = max(1, round(slope_len / (SLOPE_LEN / COURSES)))
    expo = slope_len / courses
    for c in range(courses):
        x = rnd.uniform(0.6, 12.0)
        while x < length - 4:
            w = rnd.uniform(18.0, 32.0)
            x1 = min(x + w, length - 0.6)
            s0 = c * expo - rnd.uniform(0.0, 5.0)
            s1 = min(c * expo + SHINGLE_LEN, slope_len + 18.0)
            lift = SHINGLE_LIFT + rnd.uniform(-0.8, 0.8)
            slope_box(b, x, x1, s0, s1, BOARD + lift, BOARD + lift + SHINGLE_T, MAT_SHINGLE,
                      shingle_uv(rnd.random(), rnd.random()), h1_top=BOARD + SHINGLE_T, h0_top=BOARD)
            x = x1 + 1.2
    if fascia:
        # 처마 끝판: 너와 끝을 가리는 경사 수직 판.
        slope_box(b, 0, length, -3.0, 0.0, -6.0, BOARD + SHINGLE_LIFT + SHINGLE_T + 2, MAT_SHINGLE, planar_uv(WOOD_UV, None))
    if barge:
        # 박공 끝판: 바깥(X=0)쪽 끝을 따라 세운 판.
        slope_box(b, -4.0, 0.0, -4.0 if fascia else 0.0, slope_len + 14.0, -12.0, BOARD + SHINGLE_LIFT + SHINGLE_T + 4, MAT_SHINGLE,
                  planar_uv(WOOD_UV, None))
    return b.finish()


def build_ridge(name, length):
    """용마루 덮개. 피벗 = 양쪽 경사판 밑면이 만나는 꼭짓점. 뒤집은 V자 두 판."""
    b = Builder(name)
    h = BOARD + SHINGLE_LIFT + SHINGLE_T  # 너와 윗면 높이(밑면 기준)
    for side in (1, -1):
        # t = 꼭짓점에서 경사 따라 내려간 거리, hh = 밑면에서 수직 거리. 반대편과 겹치도록 t -6부터.
        def fr(t, x, hh, side=side):
            return (x, side * (t + hh) * S45, (hh - t) * S45)
        ring_b = [fr(-6, 0, h), fr(-6, length, h), fr(32, length, h), fr(32, 0, h)]
        ring_t = [fr(-6, 0, h + 4), fr(-6, length, h + 4), fr(32, length, h + 4), fr(32, 0, h + 4)]
        b.solid(ring_b, ring_t, MAT_SHINGLE, planar_uv(WOOD_UV, None))
        b.ucx(ring_b, ring_t)
    return b.finish()


def trim_band_uv(p, n):
    # SM_Wall_Tavern_C 윗단 세로 판자: U 300cm당 1, V 0.502~0.996이 높이 143.
    return (p[0] / 300.0, 0.502 + (p[2] / 150.0) * 0.494)


def plaster_uv(p, n):
    # SM_Wall_Tavern_D 회벽과 같은 재질. 300cm당 반복 1회로 투영.
    return (p[0] / 300.0, p[2] / 300.0)


def build_gable(name, tri, mat=MAT_TRIM, uvfn=None):
    """박공 블록 150×150. 두께는 벽 돌면(Y -40~0)과 같게. tri면 (0,0)-(150,150) 빗변 아래 삼각형."""
    uvfn = uvfn or trim_band_uv
    b = Builder(name)
    y0, y1 = -40.0, 0.0
    if tri:
        prof = [(0, 0), (150, 0), (150, 150)]
    else:
        prof = [(0, 0), (150, 0), (150, 150), (0, 150)]
    bottom = [(x, y0, z) for x, z in prof]
    top = [(x, y1, z) for x, z in prof]
    b.solid(bottom, top, mat, uvfn)
    b.ucx(bottom, top)
    return b.finish()


def build_eave_fill(name):
    """처마 쪽 벽 위 쐐기. 벽 바깥 윗모서리(Y -44, Z 0)에서 45도로 벽 안쪽 끝(Y 6)까지, 지붕 밑면에 붙는다."""
    b = Builder(name)
    prof = [(-44.0, 0.0), (6.0, 0.0), (6.0, 50.0)]
    bottom = [(0.0, y, z) for y, z in prof]
    top = [(300.0, y, z) for y, z in prof]
    b.solid(bottom, top, MAT_SHINGLE, planar_uv(WOOD_UV, None))
    b.ucx(bottom, top)
    return b.finish()


def export(objs, fname):
    bpy.ops.object.select_all(action="DESELECT")
    for o in objs:
        o.select_set(True)
    bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, fname + ".fbx"), use_selection=True, apply_unit_scale=False, global_scale=1.0,
                             apply_scale_options="FBX_SCALE_NONE", mesh_smooth_type="FACE", add_leaf_bones=False,
                             bake_anim=False, axis_forward="-Z", axis_up="Y")


def main():
    os.makedirs(OUT, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = "METRIC"
    sc.unit_settings.scale_length = 1.0
    parts = [
        ("SM_GuildRoof_Slope", lambda n: build_slope(n, 300.0, SLOPE_LEN, 11)),
        ("SM_GuildRoof_SlopeVerge", lambda n: build_slope(n, 75.0, SLOPE_LEN, 12, barge=True)),
        ("SM_GuildRoof_Eave", lambda n: build_slope(n, 300.0, 100.0 / S45, 13, fascia=True)),
        ("SM_GuildRoof_EaveVerge", lambda n: build_slope(n, 75.0, 100.0 / S45, 14, barge=True, fascia=True)),
        ("SM_GuildRoof_Ridge", lambda n: build_ridge(n, 300.0)),
        ("SM_GuildRoof_GableSquare", lambda n: build_gable(n, False)),
        ("SM_GuildRoof_GableTri", lambda n: build_gable(n, True)),
        ("SM_GuildRoof_GableSquare_Plaster", lambda n: build_gable(n, False, MAT_PLASTER, plaster_uv)),
        ("SM_GuildRoof_GableTri_Plaster", lambda n: build_gable(n, True, MAT_PLASTER, plaster_uv)),
        ("SM_GuildRoof_EaveFill", lambda n: build_eave_fill(n)),
    ]
    for name, fn in parts:
        for o in list(bpy.data.objects):
            bpy.data.objects.remove(o)
        objs = fn(name)
        export(objs, name)
        tris = sum(len(p.vertices) - 2 for p in objs[0].data.polygons)
        print("PART", name, "tris", tris, "hulls", len(objs) - 1)


main()
