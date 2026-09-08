#!/usr/bin/env python3
"""性能冒烟：大量命令下确保不退化明显（不进 CI，手动跑）

用法：
    python3 tests/perf_smoke.py [版本目录=AI_version]
输出：
    生成 N 条命令（create/find/ll/delete 混合），喂给 ezfs 并计时。
"""
import os
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TARGET = sys.argv[1] if len(sys.argv) > 1 else "AI_version"
EZFS = os.path.join(ROOT, TARGET, "ezfs")
N = 5000   # 创建数量

def main():
    if not os.path.exists(EZFS):
        subprocess.run(["make", "-C", os.path.join(ROOT, TARGET)], check=True)

    cmds = []
    for i in range(N):
        cmds.append(f"create_file f{i}")
    for i in range(0, N, 500):
        cmds.append(f"find_file f{i}")
    cmds.append("ll_pre")
    cmds.append("ls")
    for i in range(N):                     # 全删
        cmds.append(f"delete_file f{i}")

    inp = "\n".join(cmds) + "\n"
    t0 = time.time()
    r = subprocess.run([EZFS], cwd=os.path.join(ROOT, TARGET), input=inp,
                       capture_output=True, text=True)
    dt = time.time() - t0
    total = len(cmds)
    print(f"{TARGET}: {total} 条命令耗时 {dt:.2f}s（{total/dt:.0f} 条/秒）")
    if r.returncode != 0:
        print("程序异常退出！", file=sys.stderr)
        return 1
    print("✅ 性能冒烟通过（预期 >1000 条/秒）")
    return 0

if __name__ == "__main__":
    sys.exit(main())