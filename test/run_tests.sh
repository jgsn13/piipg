#!/usr/bin/env bash
set -euo pipefail

export PATH="/usr/lib/postgresql/16/bin:$PATH"

echo "=========================================================="
echo " Building and Installing pg_pii_audit Extension           "
echo "=========================================================="

make clean all
make install

echo "=========================================================="
echo " Starting Isolated PostgreSQL Test Cluster               "
echo "=========================================================="

export PGDATA="/tmp/pg_test_cluster"
rm -rf "$PGDATA"

mkdir -p /tmp/pg_test_cluster
chown -R postgres:postgres /tmp/pg_test_cluster

su - postgres -c "export PATH=/usr/lib/postgresql/16/bin:\$PATH && initdb -D $PGDATA --auth=trust --no-instructions >/dev/null"
su - postgres -c "export PATH=/usr/lib/postgresql/16/bin:\$PATH && pg_ctl -D $PGDATA -l /tmp/postgres.log start"

# Wait for postgres to accept connections
for i in {1..30}; do
    if su - postgres -c "export PATH=/usr/lib/postgresql/16/bin:\$PATH && pg_isready" >/dev/null 2>&1; then
        break
    fi
    sleep 0.5
done

echo "=========================================================="
echo " Running Automated Test Suite: test_audit.sql             "
echo "=========================================================="

su - postgres -c "export PATH=/usr/lib/postgresql/16/bin:\$PATH && psql -v ON_ERROR_STOP=1 -f /workspace/test/test_audit.sql postgres"

echo "=========================================================="
echo " Shutting Down Test Cluster                              "
echo "=========================================================="

su - postgres -c "export PATH=/usr/lib/postgresql/16/bin:\$PATH && pg_ctl -D $PGDATA stop -m immediate >/dev/null 2>&1 || true"
rm -rf "$PGDATA"

echo "=========================================================="
echo " All Tests Passed Successfully!                          "
echo "=========================================================="
