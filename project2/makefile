#
# SC3020
# Project 2
# Makefile
# 

.PHONY: all

all: lint fmt

lint:
	codespell
	black --check .
	isort --check .
	ruff check
	
fmt:
	black .
	isort .
