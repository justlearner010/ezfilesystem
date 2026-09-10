#ifndef KMP_H
#define KMP_H

/* KMP 子串匹配模块（AI_version 新增，替代 human 版的 strstr）
 *
 * 匹配语义：判断文本 text 是否"包含"模式串 pat（子串匹配，非全等）。
 * 空模式串匹配一切（与 strstr(name, "") 语义一致）。
 */

/* 求模式串 pat 的 nextval 数组（优化版 next）：
 *   next[i] = 当第 i 个字符失配时，模式串指针应回退到的位置
 *   nextval 在 next 基础上跳过"与失配字符相同"的自移动，减少无谓回溯（指导书要求）。
 * 调用方保证 next 至少 (strlen(pat) + 1) 个 int。
 */
void kmp_nextval(const char *pat, int *next);

/* text 包含 pat 返回 1，否则返回 0 */
int kmp_match(const char *text, const char *pat);

#endif /* KMP_H */
