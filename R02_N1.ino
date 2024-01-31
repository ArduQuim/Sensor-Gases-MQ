#define pinoSensor A0 // Define o pino do Arduino que recebera os dados do sensor de gás
#define pinoLED 6 // Define o pino do Arduino que controlará o LED 
#define pinoBuzzer 7 // Define o pino do Arduino que controlará o buzzer 

int gas; // Variável que armazenará os dados vindos do sensor
int gasPadrao = 0; // Variável que armazenará a presença de gás presente no ambiente
int numeroAmostrasReferencia = 5; // Variável que armazenará o número de dados coletados para calcular a presença de gás no ambiente
float limiarDeAtivacaoPercentual = 20; // Variável que apontará o percentual, sobre o valor do gás ambiente, necessária para detecção do gás
int gasLimiar; // Variável que armazenará o valor limite para que o gás possa ser detectado

int frequenciaBuzzer; // Variável que armazena a frequência do som a ser emitido pelo buzzer

boolean LEDeBuzzerVariaveis = true; // Aponta se o LED e o buzzer devem funcionar de forma estática ou variável
// Caso o valor seja "false", o LED e o buzzer exibirão sempre uma luz com mesma intensidade e um som com a mesma frequência
// Caso o valor seja "true", a luz emitida pelo LED e a frequência emitida pelo buzzer variarão conforme a intensidade detectada do gás 

// Variáveis para a manipulação da intensidade do LED
int LEDAux, LEDValor;
float LEDSen;

// Comandos executados ao inicializar o Arduino
void setup() { 

  Serial.begin(9600); // Inicia a porta serial com velocidade de 9600

  pinMode(pinoSensor, INPUT); // Declara o pino do sensor para entrada de dados
  pinMode(pinoLED, OUTPUT); // Declara o pino do LED como saída

  // A seguir, são coletadas amostras de referência e, com base nelas, é calculada a presença de gás ambiente/padrão
  Serial.print("Coletando amostras de referência");
  // É executado um loop para coletar as amostras, quantas forem descritas pela variável "numeroAmostrasReferencia"
  for(int i=0; i<numeroAmostrasReferencia; i++){
    gasPadrao += analogRead(pinoSensor); // É feita a leitura do valor vindo do sensor, que é acumulado na variável "gasPadrao"
    Serial.print('.');
    delay(500); // É adicionado um intervalo entre as coletas
  }
  gasPadrao /= numeroAmostrasReferencia; // É calculada a média entre os valores obtidos

  // A função map(x, A, B, C, D) transpõe a variável x, presente no intervalo númerico limitado por A e B, para o interalo de C a D.
  // Nesse caso, a função transpões a variável "gasPadrão", lido pela porta analógica como um valor entre 0 e 1023, e o reinterpreta para uma nova escala, com valores de 0 a 100 
  // Por fim, o novo valor obtido é atribuído à mesma variável "gasPadrão"
  gasPadrao = map(gasPadrao,0,1023,0,100);

  // A presença de gás ambiente/padrão é exibida no monitor serial
  Serial.println();
  Serial.print("Presença de gás inicial (escala de 0 a 100): ");
  Serial.println(gasPadrao);

  // Para que seja possível atestar a presença de um gás, é necessário observar uma mudança significativa no valor de gás obtido através do sensor
  // Assim, a variável "gasLimiar" soma à variável "gasPadrão" uma porcentagem oriunda da variável "limiarDeAtivacaoPercentual"
  // Dessa forma, caso o valor obtido pelo sensor ultrapasse o valor de "gasLimiar", é atestada a presença de determinado gás
  gasLimiar = gasPadrao * (1+(limiarDeAtivacaoPercentual/100));

  // A fim de evitar erros, foram adicionadas algumas condições de existência para o valor "gasLimiar"
  if(gasLimiar < 15){ // Caso "gasLimiar" seja menor que 15
    gasLimiar = 15; // "gasLimiar" passa a ser 15
  }else if(gasLimiar > 90){ // Caso "gasLimiar" seja maior que 95
    gasLimiar = 90; // "gasLimiar" passa a ser 95
  }

  // O valor limite de gás para a ativação do sistema é exibido na porta serial
  Serial.print("Limiar de presença de gás (escala de 0 a 100): ");
  Serial.println(gasLimiar);
}

// Comandos que serao executados repetidamente (loop)
void loop(){

  gas = analogRead(pinoSensor); // Lê o pino analógico e armazena o valor recebido

  gas = map(gas,0,1023,0,100); // Reinterpreta os dados obtidos, que variam de 0 a 1023 e passam a variar entre 0 e 100

  // Os proximos comandos imprimem a presença de gás na porta serial
  Serial.print("Presença de gás (escala de 0 a 100):"); 
  Serial.print(gas); // Imprime a presença de gás obtida

  // O resultado fica como:
  /*
  Presença de gás: 30 unidades
  */

  if(gas > gasLimiar){ // Caso seja detectada a presença de determinado gás, o seguinte bloco é excutado
  
    Serial.print(" - Gás detectado!"); // Imprime um aviso no monitor serial, indicando a detecção de certo gás

    if(LEDeBuzzerVariaveis == 0){ // Caso o usuário deseja que o LED e o buzzer sejam estáticos
      
      frequenciaBuzzer = 1000; // Frequência, em Hertz (Hz) do som a ser emitido pelo buzzer
      tone(pinoBuzzer, frequenciaBuzzer); // Faz com que o buzzer, ligado ao "pinoBuzzer", emita um som de frequência "frequenciaBuzzer"
    
      digitalWrite(pinoLED, HIGH); // Liga o LED conectado ao "pinoLED"    
      
    }else{ // Caso o usuário deseja que o LED e o buzzer variem conforme a mudança na quantidade de gás detectada
      
      // Frequência, em Hertz (Hz) do som a ser emitido pelo buzzer
      frequenciaBuzzer = map(gas, gasLimiar, 100, 80, 3000); // Associa à variável "frequenciaBuzzer" um valor de frequência, proporcional ao intervalo entre "gasLimiar" e "gas", e que varia entre 80 e 3000 Hertz (Hz)
      tone(pinoBuzzer, frequenciaBuzzer); // Faz com que o buzzer, ligado ao "pinoBuzzer", emita um som de frequência "frequenciaBuzzer"
  
      // Intensidade, entre 0 e 255, a ser aplicada no LED
      LEDAux = map(gas, gasLimiar, 100, 1, 89); // Associa à variável "LED" um valor de ângulo, proporcional ao intervalo entre "gasLimiar" e "gas", e que varia entre 1° e 90°
      LEDSen = (sin(LEDAux*(3.1412/180))); // Converte esse ângulo de graus para radianos e calcula o seno desse ângulo correspondente
      LEDValor = int(LEDSen*255); // Com base no seno, que varia de 0 a 1, esse valor é multiplicado por 255 para que esse valor varie de 0 a 255
      // O método "analogWrite", quando aplicado numa porta digital que suporta a tecnologia PWM (tais portas possuem o símbolo '~' ao lado do número correspondente), permitem a emissão de sinais com níveis que vairam de 0 a 255
      analogWrite(pinoLED, LEDValor); // Assim, o valor de intensidade obtido anteriormente ("LEDValor"), que varia de 0 a 255, é aplicado ao pino "pinoLED", controlando a intensidade da luz emitida pelo LED
    
    }
  }else{ // Caso não seja detectada a presença de determinado gás, o seguinte bloco é executado

    noTone(pinoBuzzer); // Desliga o buzzer conectado ao "pinoBuzzer"
    digitalWrite(pinoLED, LOW); // Desliga o LED conectado ao "pinoLED"
  
  }

  Serial.println(); // Imprime uma quebra de linha no monitor serial

  delay(3000); // Estabelece um intervalo em milisegundos entre as medicoes (3000 ms = 3 s)
}
