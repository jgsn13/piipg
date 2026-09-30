# Projeto TCC - Extensao PostgreSQL em C (PII Detector)

Projeto simples pro TCC pra mostrar que da pra interceptar queries de INSERT no postgres usando os hooks internos em C, sem precisar ficar criando trigger nas tabelas.

A ideia aqui e basica: quando roda um INSERT qualquer no banco, o hook em C intercepta a execucao, olha o que ta sendo inserido em cada coluna de texto, roda uma regex pra ver se acha email e calcula a probabilidade, gravando tudo na tabela `pii_audit.identified_table_columns`.

## Como funciona (resumo)

Ao inves de criar trigger em cada tabela do usuario, usei o `ExecutorRun_hook` direto do postgres:

1. Carrega a extensao com `CREATE EXTENSION pg_pii_audit;` e da `LOAD 'pg_pii_audit';`.
2. O postgres chama a funcao `_PG_init` da nossa lib em C que coloca a nossa funcao no `ExecutorRun_hook`.
3. Toda query que roda no banco passa por ali. A gente deixa rodar o comando normal (`standard_ExecutorRun`) e checa se a operacao foi um `CMD_INSERT`.
4. Se for insert, o codigo pega a arvore do plano (`planTree`), acha os nomes das colunas e os valores que tao entrando.
5. Testa os valores com regex basica de email (`is_email`).
6. Grava ou atualiza os contadores via SPI na tabela `identified_table_columns`, calculando a probabilidade (`matched_samples / total_samples`).

## Estrutura dos arquivos

- `src/pg_pii_audit.c`: codigo em C com o hook e a gravacao via SPI (~130 linhas bem direto).
- `sql/pg_pii_audit--1.0.sql`: cria a tabela `identified_table_columns`.
- `Makefile`: makefile padrao usando o PGXS do postgres.
- `test/test_audit.sql`: script simples de teste com insert pra demonstrar.
- `test/run_tests.sh`: script que sobe o cluster de teste no docker e roda tudo.

## Como compilar e rodar

Deixei um container com postgres 16 configurado pra compilar e testar sem sujar a maquina:

```bash
# rodar os testes direto pelo docker
docker run --rm -v (pwd):/workspace:z -w /workspace pg_pii_dev bash /workspace/test/run_tests.sh
```

Ou no psql:

```sql
CREATE EXTENSION pg_pii_audit;
LOAD 'pg_pii_audit';

-- cria uma tabela normal de teste (nao precisa de trigger)
CREATE TABLE clientes (nome text, email text, cidade text);

-- faz os inserts
INSERT INTO clientes VALUES ('Alice', 'alice@empresa.com', 'SP');
INSERT INTO clientes VALUES ('Bob', 'sem_email', 'RJ');

-- consulta o que o hook pegou
SELECT table_name, column_name, total_samples, matched_samples, column_probability 
FROM pii_audit.identified_table_columns;
```
