#!/usr/bin/env python3
"""ezfilesystem 测试运行器

用法（仓库根目录或任意位置）：
    python3 tests/run_tests.py [版本目录]

版本目录：默认 human_version，可传 AI_version（支持两个版本共享同一套用例回归）。
流程：
1. make 编译目标版本（要求 gcc + make）
2. 遍历 tests/cases/*.in，喂给 ./ezfs
3. 归一化程序输出（去 ">> " 提示符前缀、去空行/尾随提示符），
   与同名 .expected 期望文件逐行对比
退出码：全过 0，有失败 1（供 CI 使用）
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TARGET = sys.argv[1] if len(sys.argv) > 1 else "human_version"
EZFS_DIR = os.path.join(ROOT, TARGET)
CASES_DIR = os.path.join(ROOT, "tests", "cases")
EZFS = os.path.join(EZFS_DIR, "ezfs")


def build() -> bool:
    r = subprocess.run(["make", "-C", EZFS_DIR], capture_output=True, text=True)
    if r.returncode != 0:
        print("编译失败：", file=sys.stderr)
        print(r.stdout + r.stderr, file=sys.stderr)
        return False
    return True


def normalize(out: str) -> list:
    """程序输出 → 纯结果行序列：
    - 去掉 ">> " 提示符前缀（cd 无输出时可能出现连续多个）
    - 去掉空行（EOF 前的尾随提示符产生）
    """
    lines = []
    for ln in out.splitlines():
        while ln.startswith(">> "):
            ln = ln[3:]
        if ln:
            lines.append(ln)
    return lines


def main() -> int:
    if not build():
        return 1
    passed = failed = 0
    for name in sorted(os.listdir(CASES_DIR)):
        if not name.endswith(".in"):
            continue
        base = name[:-3]
        exp_path = os.path.join(CASES_DIR, base + ".expected")
        if not os.path.exists(exp_path):
            continue
        with open(os.path.join(CASES_DIR, name)) as f:
            r = subprocess.run([EZFS], cwd=EZFS_DIR, stdin=f,
                               capture_output=True, text=True)
        got = normalize(r.stdout)
        exp = normalize(open(exp_path).read())
        if got == exp:
            print(f"PASS  {base}")
            passed += 1
            continue
        print(f"FAIL  {base}")
        for i in range(max(len(got), len(exp))):
            a = got[i] if i < len(got) else "<EOF>"
            b = exp[i] if i < len(exp) else "<EOF>"
            if a != b:
                print(f"  第{i}行  实际: {a!r}  期望: {b!r}")
        failed += 1
    print(f"----\npassed={passed} failed={failed}")
    if passed + failed == 0:
        print(f"警告：一个用例都没跑（期望文件缺失？），按失败处理", file=sys.stderr)
        return 1
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())