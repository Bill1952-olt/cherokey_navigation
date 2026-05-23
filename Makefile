# Branche distante
BRANCH = main

# ---------------------------
# Ajouter un fichier précis, commit et push
# make deploy file="fichier_nom" m="commit"

# ---------------------------
deploy:
	@if [ -z "$(file)" ]; then \
		echo "Erreur : veuillez spécifier file=<nom_du_fichier>"; \
		exit 1; \
	fi
	@if [ -z "$(m)" ]; then \
		echo "Erreur : veuillez spécifier m=\"message de commit\""; \
		exit 1; \
	fi
	git add $(file)
	git commit -m "$(m)" || echo "Rien à committer"
	git pull --rebase origin $(BRANCH)
	git push origin $(BRANCH)
