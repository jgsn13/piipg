\set ECHO all
\x off

-- cria extensao e da o load na biblioteca em C
DROP EXTENSION IF EXISTS pg_pii_audit CASCADE;
CREATE EXTENSION pg_pii_audit;
LOAD 'pg_pii_audit';

-- cria uma tabela qualquer de teste (sem trigger nenhum)
DROP TABLE IF EXISTS clientes CASCADE;
CREATE TABLE clientes (
    id serial PRIMARY KEY,
    nome text,
    email text,
    cidade text
);

-- inserindo dados normais pra testar
-- esse aqui tem email
INSERT INTO clientes (nome, email, cidade) 
VALUES ('Alice Silva', 'alice@empresa.com', 'São Paulo');

-- esse aqui nao tem email
INSERT INTO clientes (nome, email, cidade) 
VALUES ('Bob Santos', 'sem_email', 'Curitiba');

-- olhando o que o hook salvou na tabela
SELECT 
    table_name,
    column_name,
    pattern_type,
    total_samples,
    matched_samples,
    column_probability,
    table_probability
FROM pii_audit.identified_table_columns;
