/*
 * extensao em c pro tcc - observabilidade de dados sensiveis
 * ideia: usar o hook interno do postgres (ExecutorRun_hook) pra ver os inserts
 * e salvar o que achar na tabela identified_table_columns
 */

#include "postgres.h"
#include "fmgr.h"
#include "executor/executor.h"
#include "executor/spi.h"
#include "nodes/plannodes.h"
#include "nodes/primnodes.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "catalog/pg_type.h"
#include <regex.h>

PG_MODULE_MAGIC;

// guarda o hook original do postgres pra chamar ele depois
static ExecutorRun_hook_type prev_ExecutorRun = NULL;

// trava pra nao dar loop infinito quando fizer o insert pelo spi
static bool in_hook = false;

// funcao simples com regex pra checar se a string tem formato de email
static bool
is_email(const char *str)
{
    regex_t reg;
    bool match = false;

    if (!str || !*str)
        return false;

    // regex basica de email
    if (regcomp(&reg, "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", REG_EXTENDED | REG_ICASE) == 0)
    {
        match = (regexec(&reg, str, 0, NULL, 0) == 0);
        regfree(&reg);
    }
    return match;
}

// aqui e o hook onde passa toda execucao de query no banco
static void
pii_audit_ExecutorRun(QueryDesc *queryDesc,
                      ScanDirection direction,
                      uint64 count,
                      bool execute_once)
{
    // primeiro executa a query normal pro banco funcionar direito
    if (prev_ExecutorRun)
        prev_ExecutorRun(queryDesc, direction, count, execute_once);
    else
        standard_ExecutorRun(queryDesc, direction, count, execute_once);

    // so interessa se for comando de INSERT
    if (!in_hook && queryDesc->operation == CMD_INSERT && queryDesc->plannedstmt && queryDesc->plannedstmt->planTree)
    {
        // pega a tabela que ta recebendo o insert
        RangeTblEntry *rte = (RangeTblEntry *) linitial(queryDesc->plannedstmt->rtable);
        char *table_name = get_rel_name(rte->relid);
        Plan *sub = queryDesc->plannedstmt->planTree->lefttree;

        // se for a propria tabela do modulo ignora pra nao dar loop
        if (!table_name || strcmp(table_name, "identified_table_columns") == 0 || !sub || !sub->targetlist)
            return;

        in_hook = true;
        // abre conexao interna do spi pra gravar os dados
        if (SPI_connect() == SPI_OK_CONNECT)
        {
            ListCell *lc;
            // olha coluna por coluna do insert
            foreach(lc, sub->targetlist)
            {
                TargetEntry *tle = (TargetEntry *) lfirst(lc);
                if (tle->expr && IsA(tle->expr, Const))
                {
                    Const *c = (Const *) tle->expr;
                    // so olha tipo texto / varchar
                    if (!c->constisnull && (c->consttype == TEXTOID || c->consttype == VARCHAROID || c->consttype == BPCHAROID))
                    {
                        char *val = TextDatumGetCString(c->constvalue);
                        bool matched = is_email(val);

                        // grava ou atualiza a probabilidade na tabela
                        char *sql = psprintf(
                            "INSERT INTO pii_audit.identified_table_columns "
                            "    (table_name, column_name, pattern_type, total_samples, matched_samples, column_probability, table_probability) "
                            "VALUES ('%s', '%s', 'EMAIL', 1, %d, %s, %s) "
                            "ON CONFLICT (table_name, column_name, pattern_type) DO UPDATE SET "
                            "    total_samples = pii_audit.identified_table_columns.total_samples + 1, "
                            "    matched_samples = pii_audit.identified_table_columns.matched_samples + EXCLUDED.matched_samples, "
                            "    column_probability = ROUND(((pii_audit.identified_table_columns.matched_samples + EXCLUDED.matched_samples)::numeric / "
                            "                         (pii_audit.identified_table_columns.total_samples + 1)::numeric), 2)::float8, "
                            "    table_probability = ROUND(((pii_audit.identified_table_columns.matched_samples + EXCLUDED.matched_samples)::numeric / "
                            "                        (pii_audit.identified_table_columns.total_samples + 1)::numeric), 2)::float8;",
                            table_name,
                            tle->resname ? tle->resname : "coluna",
                            matched ? 1 : 0,
                            matched ? "1.0" : "0.0",
                            matched ? "1.0" : "0.0"
                        );

                        SPI_exec(sql, 0);
                        pfree(sql);
                        pfree(val);
                    }
                }
            }
            SPI_finish();
        }
        in_hook = false;
    }
}

// registra o hook quando carrega a extensao
void _PG_init(void);
void
_PG_init(void)
{
    prev_ExecutorRun = ExecutorRun_hook;
    ExecutorRun_hook = pii_audit_ExecutorRun;
}

// desregistra quando descarregar
void _PG_fini(void);
void
_PG_fini(void)
{
    ExecutorRun_hook = prev_ExecutorRun;
}
