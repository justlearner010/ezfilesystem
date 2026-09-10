#!/bin/bash
# ============================================================
#  简单文件系统 —— 一键运行脚本（macOS 双击 / 终端执行）
#
#  用法：
#    macOS：双击本文件（若提示"无法打开"，请右键 → 打开）
#    终端：./run.sh
#
#  功能：编译并运行 human 版 / AI 版，可回放官方样例、
#        交互式体验、跑全部测试用例、对比两版输出。
# ============================================================

cd "$(dirname "$0")" || exit 1
REPO="$(pwd)"

GRN='\033[0;32m'; YEL='\033[0;33m'; RED='\033[0;31m'; CYA='\033[0;36m'; NC='\033[0m'
hr() { echo "------------------------------------------------------------"; }
big_hr() { echo "============================================================"; }

# ---- 环境检查：C 编译器 ----
if   command -v gcc   >/dev/null 2>&1; then CC=gcc
elif command -v cc    >/dev/null 2>&1; then CC=cc
elif command -v clang >/dev/null 2>&1; then CC=clang
else
  echo -e "${RED}✗ 未检测到 C 编译器（gcc / cc / clang）。${NC}"
  echo
  echo "  请先安装："
  echo "    macOS         →  xcode-select --install"
  echo "    Ubuntu/Debian →  sudo apt install build-essential"
  echo
  read -rsp "按任意键退出..."; echo; exit 1
fi
HAVE_MAKE=$(command -v make >/dev/null 2>&1 && echo yes || echo no)
HAVE_PY=$(command -v python3 >/dev/null 2>&1 && echo yes || echo no)

# ---- 编译指定版本 ----
build() {
  local ver="$1"
  echo -e "${CYA}>> 编译 ${ver} ...${NC}"
  ( cd "$REPO/$ver" || return 1
    rm -f ezfs
    if [ "$HAVE_MAKE" = yes ] && make >/dev/null 2>&1; then
      :
    else
      "$CC" -std=c11 -Wall -Wextra -g -Iinclude -o ezfs src/*.c || return 1
    fi
    [ -x ./ezfs ] || return 1
  ) || { echo -e "${RED}✗ ${ver} 编译失败。${NC}"; return 1; }
  echo -e "${GRN}✓ ${ver} 编译成功${NC}"
}

# ---- 回放官方样例 ----
run_sample() {
  build "$1" || return 1
  echo
  echo -e "${CYA}>> 回放官方样例 tests/sample_input.txt${NC}"
  big_hr
  ( cd "$REPO/$1" && ./ezfs < ../tests/sample_input.txt )
  big_hr
  echo -e "${GRN}（以上输出应与报告第 4 节的终端截图一致）${NC}"
}

# ---- 交互式体验 ----
run_interactive() {
  build "$1" || return 1
  echo
  echo "------------------------------------------------------------"
  echo "  已进入交互模式，直接输入命令回车即可。"
  echo "  退出：Ctrl+D"
  echo
  echo "  可用命令："
  echo "    create_file f        create_dir d        ls"
  echo "    cd d / cd ..         ll_pre / ll_post    find_file kw"
  echo "    open f               write_file \"内容\"   read_file f"
  echo "    close_file           delete_file f       delete_dir d"
  echo "    rename_file old new  rename_dir old new"
  echo "------------------------------------------------------------"
  ( cd "$REPO/$1" && ./ezfs )
}

# ---- 全部测试用例 ----
run_all_tests() {
  build "$1" || return 1
  echo
  if [ "$HAVE_PY" = yes ]; then
    python3 tests/run_tests.py "$1"
  else
    echo -e "${YEL}! 未检测到 python3，跳过自动化测试。${NC}"
    echo "  安装 python3 后可运行：python3 tests/run_tests.py $1"
  fi
}

# ---- 对比两版输出 ----
compare_versions() {
  build human_version || return 1
  build AI_version    || return 1
  echo
  ( cd "$REPO/human_version" && ./ezfs < ../tests/sample_input.txt ) > /tmp/ezfs_human.out 2>&1
  ( cd "$REPO/AI_version"    && ./ezfs < ../tests/sample_input.txt ) > /tmp/ezfs_ai.out   2>&1
  if diff -q /tmp/ezfs_human.out /tmp/ezfs_ai.out >/dev/null 2>&1; then
    echo -e "${GRN}✓ 两版对官方样例的输出完全一致（外部行为等价）。${NC}"
  else
    echo -e "${YEL}! 两版输出存在差异：${NC}"
    diff -u /tmp/ezfs_human.out /tmp/ezfs_ai.out | head -40
  fi
  echo
  echo "human 版输出："; hr; cat /tmp/ezfs_human.out
}

# ---- 二级菜单 ----
sub_menu() {
  local ver="$1" ver_name="$2"
  while true; do
    echo
    big_hr
    echo -e "  已选择：${GRN}${ver_name}${NC}"
    big_hr
    echo "  [1] 回放官方样例（推荐，对照报告第 4 节截图）"
    echo "  [2] 交互式体验（手动输入命令）"
    echo "  [3] 运行全部测试用例（自动比对期望输出）"
    echo "  [0] 返回上一级"
    echo
    read -rp "请输入编号 [0-3]：" sub || exit 0
    case "$sub" in
      1) run_sample      "$ver" ;;
      2) run_interactive "$ver" ;;
      3) run_all_tests   "$ver" ;;
      0) return 0 ;;
      *) echo -e "${RED}无效输入，请重新选择。${NC}" ;;
    esac
  done
}

# ---- 主菜单 ----
while true; do
  clear 2>/dev/null || true
  big_hr
  echo -e "        ${CYA}简单文件系统 —— 一键运行${NC}"
  echo "        （数据结构课程设计 · 202532110113 刘亦轩）"
  big_hr
  echo "  [1] human 版（自主实现的简化版）"
  echo "  [2] AI 版（升级版·最终版）"
  echo "  [3] 对比两版输出（同一输入，逐行对照）"
  echo "  [0] 退出"
  echo
  read -rp "请输入编号 [0-3]：" choice || exit 0
  case "$choice" in
    1) sub_menu human_version "human 版（简化版）" ;;
    2) sub_menu AI_version    "AI 版（升级版）"    ;;
    3) compare_versions ;;
    0) echo "再见！"; exit 0 ;;
    *) echo -e "${RED}无效输入，请重新选择。${NC}"; sleep 1 ;;
  esac
  echo
  read -rsp "按任意键继续..."; echo
done
