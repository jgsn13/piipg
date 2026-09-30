-- script sql da extensao
\echo Use "CREATE EXTENSION pg_pii_audit" to load this file. \quit

-- tabela onde vai ficar salvo os dados encontrados e a probabilidade
CREATE TABLE identified_table_columns (
    id serial PRIMARY KEY,
    table_name text NOT NULL,
    column_name text NOT NULL,
    pattern_type text NOT NULL,
    total_samples integer NOT NULL DEFAULT 0,
    matched_samples integer NOT NULL DEFAULT 0,
    column_probability double precision NOT NULL DEFAULT 0.0,
    table_probability double precision NOT NULL DEFAULT 0.0,
    detected_at timestamptz DEFAULT now(),
    CONSTRAINT uq_table_col_pattern UNIQUE (table_name, column_name, pattern_type)
);
