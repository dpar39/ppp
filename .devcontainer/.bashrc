#!/bin/bash

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/" >/dev/null 2>&1 && pwd )"

export PROMPT_COMMAND='history -a' && export HISTFILE=/src/.bash_history

########################################
# General shortcuts
########################################
alias sb="source ~/.bashrc"
alias gs="git status"
alias gb="git branch"
alias gc="git checkout"
alias gf="git fetch"
alias gcan="git commit --amend --no-edit"
alias gpf="git push --force"

git_branch_name() {
    branch_name="$(git symbolic-ref HEAD 2>/dev/null)" || branch_name="(unnamed branch)"
    echo ${branch_name##refs/heads/}
}

gpup() {
    branch_name="$(git_branch_name)"
    git push -u origin $branch_name
}

gmain() {
    branch_name="$(git_branch_name)"
    git fetch origin main:main && git checkout main && git branch -d $branch_name
}

source /usr/share/bash-completion/completions/git

if [ "`id -u`" -eq 0 ]; then
    PS1="\[\033[m\]\[\033[1;35m\]\t\[\033[m\]|\[\e[1;31m\]\u\[\e[1;36m\]\[\033[m\]@\[\e[1;36m\]\h\[\033[m\]\[\e[0m\]\[\e[1;32m\][\W]> \[\e[0m\]"
else
    PS1="\[\033[m\]\[\033[1;35m\]\t\[\033[m\]|\[\e[1m\]\u\[\e[1;36m\]\[\033[m\]@\[\e[1;36m\]\h\[\033[m\]\[\e[0m\]|\e[1;32m\]\w\n$ \[\e[0m\]"
fi

sudo chmod 666 /var/run/docker.sock

# BASH standalone npm install autocomplete. Add this to ~/.bashrc file.
_npm_install_completion () {
    local words cword
    if type _get_comp_words_by_ref &>/dev/null; then
      _get_comp_words_by_ref -n = -n @ -w words -i cword
    else
      cword="$COMP_CWORD"
      words=("${COMP_WORDS[@]}")
    fi

	local si="$IFS"

	# if your npm command includes `install` or `i `
	if [[ ${words[@]} =~ 'install' ]] || [[ ${words[@]} =~ 'i ' ]]; then
		local cur=${COMP_WORDS[COMP_CWORD]}

		# supply autocomplete words from `~/.npm`, with $cur being value of current expansion like 'expre'
		COMPREPLY=( $( compgen -W "$(ls ~/.npm )" -- $cur ) )
	fi

	IFS="$si"
}
# bind the above function to `npm` autocompletion
complete -o default -F _npm_install_completion npm
## END BASH npm install autocomplete