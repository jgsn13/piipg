# 1 INTRODUÇÃO

Com a crescente digitalização dos serviços e o acúmulo de dados nas organizações,
a privacidade e o controle sobre informações pessoais tornaram-se tópicos centrais
tanto na legislação quanto na engenharia de software. Leis como a Lei Geral de
Proteção de Dados (LGPD) no Brasil e o *General Data Protection Regulation* (GDPR)
na União Europeia estabeleceram direitos fundamentais aos titulares, incluindo o
direito ao esquecimento e a exigência de que as empresas possam localizar e excluir
definitivamente todos os dados relacionados a um indivíduo quando solicitado.

Entretanto, atender a essas exigências impõe um desafio desproporcional para
pequenas e médias empresas (PMEs). Ao contrário das grandes corporações, essas
organizações geralmente não dispõem de times dedicados de engenharia de dados ou
especialistas em governança. No dia a dia de sistemas que utilizam PostgreSQL, os
dados pessoais raramente ficam restritos a tabelas convencionais de cadastro. Ao
longo da evolução da aplicação, nomes, e-mails e números de CPF acabam sendo
persistidos em tabelas de auditoria, históricos e colunas de texto com nomes genéricos,
como "body", "data" ou "payload".

As ferramentas comerciais de auditoria e governança existentes no mercado são
conhecidas por serem excessivamente complexas e caras. Além do alto custo de
licenciamento, essas soluções exigem conhecimento avançado em engenharia de dados
para instalação e manutenção, demandando a orquestração de pipelines externos e
agentes dedicados. Outro agravante é que essas abordagens não operam de maneira
integrada ao núcleo (*core*) do banco de dados, atuando apenas por meio de varreduras
periódicas que geram tráfego na rede e não oferecem visibilidade contínua.

Diante dessa lacuna, este trabalho propõe o desenvolvimento de uma solução integrada,
leve e acessível, voltada à auditoria e observabilidade de Informações de Identificação
Pessoal (*Personally Identifiable Information* - PII) diretamente no PostgreSQL. O
coração da proposta é uma extensão nativa desenvolvida em linguagem C que utiliza os
*hooks* internos do banco para inspecionar os dados no momento em que entram ou saem.
A ferramenta calcula a porcentagem de ocorrências de dados sensíveis em tabelas e
colunas, permitindo um setup simples que pode ser executado por um desenvolvedor
comum ou DBA, conferindo autonomia ao próprio motor de banco de dados.

Adicionalmente, a solução estrutura as informações de forma hierárquica para traçar
a linhagem dos dados (*data lineage*), revelando sua origem e propagação pelo esquema.
Para tornar a experiência prática para pequenas equipes, o ecossistema é complementado
por um servidor em Node.js com o framework NestJS, que sugere comandos SQL para
a remoção segura de dados, e um painel web desenvolvido em Angular, onde o usuário
consegue visualizar os dados em formato de grafos e confirmar as ações de exclusão.

## 1.1 Motivação e Justificativa

Em pequenas e médias empresas, os sistemas frequentemente operam com equipes
enxutas e prazos apertados, nos quais documentar rigorosamente cada coluna onde um dado
pessoal transita é uma tarefa inviável na prática. Quando uma notificação da autoridade
nacional de proteção de dados ou uma intimação judicial exige a exclusão dos dados de
determinado titular, a equipe técnica costuma recorrer a buscas manuais e rotinas ad-hoc.
Esse método manual é lento, falho e traz sérios riscos de deixar registros sensíveis
esquecidos em tabelas secundárias, sujeitando a empresa a pesadas sanções legais.

Embora o mercado ofereça plataformas robustas de catálogo e linhagem de dados, elas
foram concebidas para a realidade de grandes corporações com estruturas complexas de
dados. Para uma empresa de menor porte, investir em licenças onerosas e alocar recursos
humanos qualificados para manter essas ferramentas externas é uma barreira quase
intransponível. Muitas dessas soluções ainda operam fora do banco de dados, exigindo
configurações complexas de infraestrutura e credenciais de alto privilégio.

A motivação deste trabalho decorre da necessidade de democratizar o acesso à
auditoria e governança de dados pessoais. Ao integrar a detecção diretamente ao *core* do
PostgreSQL por meio de uma extensão nativa, elimina-se a dependência de arquiteturas
externas pesadas. A facilidade de uso e o setup descomplicado garantem que qualquer
desenvolvedor ou DBA consiga monitorar dados em produção, entender a linhagem dessas
informações e atender com agilidade a processos judiciais e solicitações da LGPD,
mesmo sem contar com uma equipe especializada de engenharia de dados.

## 1.2 Objetivos

A seguir são definidos o objetivo geral e os objetivos específicos do projeto.

### 1.2.1 Objetivo Geral

Desenvolver e validar uma solução acessível e de fácil configuração para auditoria,
detecção contínua e rastreamento de linhagem de dados pessoais (PII) no PostgreSQL,
composta por uma extensão nativa em C acoplada ao *core* do banco de dados, um serviço
backend em NestJS para sugestão de planos de remoção e uma interface em Angular
destinada ao gerenciamento visual por pequenas e médias empresas.

### 1.2.2 Objetivos Específicos

1. **Desenvolver a extensão nativa em C para o PostgreSQL**: criar a camada de
interceptação de comandos no nível dos executores do banco, inspecionando os dados em
tempo de execução com facilidade de instalação e baixo impacto operacional;
2. **Implementar a detecção e cálculo de métricas de PII**: elaborar rotinas para
identificar padrões de dados pessoais (como CPF e e-mail) e armazenar a porcentagem de
ocorrências desses dados em tabelas e colunas;
3. **Mapear a linhagem hierárquica com LTREE**: utilizar a extensão oficial `ltree` do
PostgreSQL para registrar o histórico de navegação e transformação dos dados sensíveis;
4. **Construir um serviço backend com NestJS**: desenvolver uma API capaz de ler os
metadados registrados pela extensão e sugerir queries otimizadas para a exclusão ou
anonimização de dados pessoais requisitados;
5. **Desenvolver um frontend interativo com Angular**: construir uma interface visual
que apresente a linhagem de dados em formato de grafos e permita ao usuário revisar e
confirmar as propostas de exclusão de dados;
6. **Validar a usabilidade e a eficácia em ambiente prático**: simular cenários de
operação em bases de dados existentes, avaliando a precisão da detecção, a facilidade de
setup para desenvolvedores e a efetividade na localização de dados dispersos.