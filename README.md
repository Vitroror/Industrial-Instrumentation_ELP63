Esse repositório consiste na atividade de carro autonomo realizada para a disciplina Instrumentação Industrial (ELP63):

# Carro_Robo_Gurizes.ino
**Descrição**: Este código é o código principal do projeto, sendo o que é programado e gravado dentro da placa Arduino Uno R3. O código é responsável por realizar a interface de todos os sensores docarro com o módulo bluetooth, que por sua vez envia os dados para ocomputador, bem como recebe informações de navegação.

# Python_control_module.py
**Descrição**: Este código é o módulo de controle do carro. Nele, o computador envia informações de navegação para o Arduino (comandos de movimento) e também recebe as informações dos sensores, imprimindo-os no pyshell, e simultaneamente registrando-os em um log que pode ser enviado para a nuvem.
