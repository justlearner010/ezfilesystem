#include "kmp.h"
#include <string.h>

#define KMP_MAX_PAT 20   /* 与 fs.h 的 NAME_SIZE 保持一致（文件名最长 19 + 结尾符） */

/* nextval 数组计算（0-based 下标，nextval[0] = -1 表示回退到起点前）
 *
 * 思路：
 *   朴素 next：next[i] = 模式串前 i 个字符的最长相等前后缀长度。
 *   优化 nextval：失配字符 pat[i] 与将要跳到的字符 pat[next[i]] 相同时，
 *   那次跳转必然再次失配，直接继承 nextval[next[i]]，跳过无谓比较。
 *   例：pat = "abab"，求得的 nextval 会在失配于 'b' 时直接回到 0 而不是再比一次 'a'。
 */
void kmp_nextval(const char *pat, int *next) {
    int m = (int)strlen(pat);
    int i = 0, j = -1;
    next[0] = -1;

    while (i < m) {
        if (j == -1 || pat[i] == pat[j]) {
            ++i; ++j;
            if (i < m && pat[i] == pat[j])   /* i==m 时不再访问 pat[i]，防越界 */
                next[i] = next[j];           /* nextval 优化：跳过相同字符的跳转 */
            else
                next[i] = j;                 /* 朴素 next */
        } else {
            j = next[j];                     /* 失配：模式串指针回退 */
        }
    }
}

/* text 包含 pat 返回 1，否则 0。
 * 文件名长度上限 NAME_SIZE=20；若模式串比最长文件名还长，必然无匹配，直接返回 0，
 * 同时保证 next 数组用固定大小也不越界。
 */
int kmp_match(const char *text, const char *pat) {
    int m = (int)strlen(pat);
    if (m == 0)
        return 1;                        /* 空模式：匹配一切（与 strstr 一致） */

    int n = (int)strlen(text);
    if (m > KMP_MAX_PAT)
        return 0;                        /* 模式比最长文件名还长，必然无匹配 */

    int next[KMP_MAX_PAT + 1];           /* m <= KMP_MAX_PAT，下标 0..m 固定够用 */
    kmp_nextval(pat, next);

    int i = 0, j = 0;
    while (i < n) {
        if (j == -1 || text[i] == pat[j]) {
            ++i; ++j;                    /* 字符相等：双双前进 */
        } else {
            j = next[j];                 /* 失配：文本不回退，模式串跳转 */
        }
        if (j == m)
            return 1;                    /* 模式串走完 = 找到子串 */
    }
    return 0;
}