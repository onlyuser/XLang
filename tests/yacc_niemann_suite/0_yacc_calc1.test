// Permission to reproduce portions of this document is given provided the web
// site listed below is referenced. No additional restrictions apply. Source
// code, when part of a software project, may be used freely without reference
// to the author.
//
// Tom Niemann
// Portland, Oregon
// epaperpress.com

%{
    #include <stdio.h>
    int yylex(void);
    void yyerror(char *);
%}

%token INTEGER

%%

program:
        program expr '\n'         { printf("%d\n", $2); }
        | 
        ;

expr:
        INTEGER
        | expr '+' expr           { $$ = $1 + $3; }
        | expr '-' expr           { $$ = $1 - $3; }
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}

int main(void) {
    yyparse();
    return 0;
}
