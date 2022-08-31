#include "irregex.h"

const char *regex_and_action[][2] = {
    { "(auto)", "auto" },
    { "(break)", "break" },
    { "(case)", "case" },
    { "(char)", "char" },
    { "(const)", "const" },
    { "(continue)", "continue" },
    { "(default)", "default" },
    { "(do)", "do" },
    { "(double)", "double" },
    { "(else)", "else" },
    { "(enum)", "enum" },
    { "(extern)", "extern" },
    { "(float)", "float" },
    { "(for)", "for" },
    { "(goto)", "goto" },
    { "(if)", "if" },
    { "(inline)", "inline" },
    { "(int)", "int" },
    { "(long)", "long" },
    { "(register)", "register" },
    { "(restrict)", "restrict" },
    { "(return)", "return" },
    { "(short)", "short" },
    { "(signed)", "signed" },
    { "(sizeof)", "sizeof" },
    { "(static)", "static" },
    { "(struct)", "struct" },
    { "(switch)", "switch" },
    { "(typedef)", "typedef" },
    { "(union)", "union" },
    { "(unsigned)", "unsigned" },
    { "(void)", "void" },
    { "(volatile)", "volatile" },
    { "(while)", "while" },
    { "(_Bool)", "_Bool" },
    { "(_Complex)", "_Complex" },
    { "(_Imaginary)", "_Imaginary" },
    { "(([A-Za-z_])(([A-Za-z_])|([0-9]))*)", "identifier" },
    { "(([/][*]([^*]|\\*[^/])*[*][/])|([/][/][^\n]*))", "comment" },
    { "((0x|0X)((([0-9a-fA-F])+)?\\.(([0-9a-fA-F])+)|(([0-9a-fA-F])+)\\.)([pP]([+-])?(([0-9])+))([flFL])?|(0x|0X)(([0-9a-fA-F])+)([pP]([+-])?(([0-9])+))([flFL])?)", "hexadecimal floating constant" },
    { "(((([0-9])+)?\\.(([0-9])+)|(([0-9])+)\\.)([Ee]([+-])?(([0-9])+))?([flFL])?|(([0-9])+)([Ee]([+-])?(([0-9])+))([flFL])?)", "decimal floating number" },
    { "(([1-9])([0-9])*((u|U)(l|L)?|(u|U)(ll|LL)|(l|L)(u|U)?|(ll|LL)(u|U)?)?)", "decimal constant" },
    { "(0([0-7])*((u|U)(l|L)?|(u|U)(ll|LL)|(l|L)(u|U)?|(ll|LL)(u|U)?)?)", "octal constant" },
    { "((0x|0X)([0-9a-fA-F])+((u|U)(l|L)?|(u|U)(ll|LL)|(l|L)(u|U)?|(ll|LL)(u|U)?)?)", "hexadecimal constant" },
    { "(L?\"([^\"\\\n]|((\\\\.)|(\\\\[0-7]{1,3})|(\\\\x([0-9a-fA-F])+)))*\")", "string literal" },
    { "(L?'([^'\\\n]|((\\\\.)|(\\\\[0-7]{1,3})|(\\\\x([0-9a-fA-F])+)))+')", "character" },
    { "(\\[|\\])", "square brackets" },
    { "(\\(|\\))", "parentheses" },
    { "(\\{|\\})", "braces" },
    { "(\\|\\|)", "logical or" },
    { "(=)", "assignment" },
    { "(&&)", "logical and" },
    { "(!)", "logical not" },
    { "(!=)", "logical ne" },
    { "(~)", "bitwise not" },
    { "(<)", "logical lt" },
    { "(<=)", "logical le" },
    { "(==)", "logical eq" },
    { "(>=)", "logical ge" },
    { "(>)", "logical gt" },
    { "(>>)", "bitwise rshift" },
    { "(\\+\\+)", "increasement" },
    { "(--)", "decreasement" },
    { "(\\+=)", "add assignment" },
    { "(-=)", "minus assignment" },
    { "(\\*=)", "multiply assignment" },
    { "(/=)", "divide assignment" },
    { "(%=)", "modulo assignment" },
    { "(-)", "minus" },
    { "(\\+)", "add" },
    { "(/)", "divide" },
    { "(%)", "modulo" },
    { "(\\?)", "question mark" },
    { "(:)", "colon" },
    { "(;)", "semicolon" },
    { "(,)", "comma" },
    { "(\\^)", "bitwise xor" },
    { "(\\|)", "bitwise or" },
    { "(\\*)", "star" },
    { "(&)", "and" },
    { "(\\.)", "dot" },
    { "(->)", "arrow" },
    { "(#(\\\\\n|[^\n])*)", "preprocessor command" },
    { "([[:space:]]+)", "" }
};

#define rule_num ((int)(sizeof(regex_and_action) / sizeof(*regex_and_action)))

NFAGraph compiled_regex[rule_num];

char *slurp(const char *filename) {
    char *buf = (char *)malloc(16);
    int bufsize = 16;
    int cnt = 0;
    int ch, peek;
    FILE *fp = fopen(filename, "r");
    assert(fp != NULL);
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\\') {
            peek = fgetc(fp);
            if (peek == '\n') {
                continue;
            }
            ungetc(peek, fp);
        }
        if (cnt == bufsize) {
            bufsize = bufsize * 2 + 4;
            buf = realloc(buf, bufsize);
            assert(buf != NULL);
        }
        buf[cnt++] = ch;
    }
    if (cnt == bufsize) {
        bufsize = bufsize * 2 + 4;
        buf = realloc(buf, bufsize);
        assert(buf != NULL);
    }
    buf[cnt++] = '\0';

    return buf;
}

int main(int argc, char *argv[]) {
    int i;
    char *pos;
    char *buf;

    assert(argc >= 2);

    for (i = 0; i < rule_num; ++i) {
        compiled_regex[i] = regex_compile(regex_and_action[i][0]);
    }

    buf = slurp(argv[1]);
    pos = buf;

    while (*pos != '\0') {
        int matched_item = -1;
        int matched_len = 0;
        int tmp;

        for (i = 0; i < rule_num; ++i) {
            int res = regex_execute(&compiled_regex[i], pos);
            if (res != -1) {
                int len = res;
                if (matched_item == -1 || len > matched_len) {
                    matched_item = i;
                    matched_len = len;
                }
            }
        }

        if (matched_item == -1)
            printf("%s\n", pos);
//        assert(matched_item != -1);
        if (matched_item == -1) {
            return 1;
        }
        tmp = pos[matched_len];
        pos[matched_len] = '\0';
        if (*regex_and_action[matched_item][1] != '\0')
            printf("%s: [%s]\n", regex_and_action[matched_item][1], pos);
        pos[matched_len] = tmp;
        pos += matched_len;
    }

    for (i = 0; i < rule_num; ++i)
        regex_free(&compiled_regex[i]);
    free(buf);

    return 0;
}
