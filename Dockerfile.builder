FROM dpar39/ppp-base:latest

# Add build user
ARG USER_NAME
ARG USER_UID
ARG USER_GID

ENV HOME=/home/$USER_NAME
RUN groupadd --gid $USER_GID $USER_NAME; exit 0
RUN useradd --uid $USER_UID --gid $USER_GID -m $USER_NAME \
  && echo $USER_NAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USER_NAME \
  && chmod 0440 /etc/sudoers.d/$USER_NAME \
  && chown -R $USER_NAME $HOME

USER ${USER_NAME}
