BIN = $(PWD)/node_modules/.bin

FMT_SRC = \
	'syntax/*.json' \
	'language/*.json' \
	'client/**/*.ts' \
	'client/**/*.js'

fmt:
	@$(BIN)/prettier --write $(FMT_SRC)
