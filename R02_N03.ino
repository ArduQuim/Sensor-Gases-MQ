// Incluir as bibliotecas necessarias
#include <LiquidCrystal_I2C.h> // Gere o display LCD
#include <SD.h> // Gere o cartão de memoria e os dados nele armazenados

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

const int LCD_linhas = 2; // Número de linhas do seu display LCD
const int LCD_colunas = 16; // Número de colunas do seu display LCD
const int numeroPiscadas = 3; // Número de vezes que o dado pisca
// As próximas 3 variáveis descrevem os intervalos de tempo empregados no display LCD e podem ser editados conforme o desejo do usuário
const float LCD_tempoAceso = 2.5; // Tempo, em segundos, em que o dado é exibido
const float LCD_tempoPiscando = 1.8; // Tempo, em segundos, em que o dado permanece piscando
const float LCD_tempoApagado = 0.7; // Intervalo, em segundos, entre a exibição dos dados

LiquidCrystal_I2C lcd(0x27,LCD_colunas,LCD_linhas); // Declara o display como um objeto chamado "lcd"

// Para o cartão SD, 3 pinos já são determinados
// SCK = 13;
// MISO = 12;
// MOSI = 11;
const int CS = 10; // Determina o ultimo pino

unsigned long tempoColeta;

byte aComAcento[8] = {
  B00010,
  B00100,
  B01110,
  B00001,
  B01111,
  B10001,
  B01110,
  B00000
};

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
  }else if(gasLimiar > 95){ // Caso "gasLimiar" seja maior que 95
    gasLimiar = 95; // "gasLimiar" passa a ser 95
  }

  // O valor limite de gás para a ativação do sistema é exibido na porta serial
  Serial.print("Limiar de presença de gás (escala de 0 a 100): ");
  Serial.println(gasLimiar);

  lcd.init(); // Inicia o display lcd(x, y), de x colunas e y linhas; Nesse caso, lcd(16,2);
  lcd.createChar(0,aComAcento); // Descreve o caracterer 'á' a ser exibido no display
  lcd.backlight(); // Liga a luz de fundo do display
  lcd.setCursor(0, 0); // Posiciona o cursor na primeira coluna (0) e na primeira linha (0)
  // Imprime, na primeira linha, o texto “Gás (0 a 100):”
  lcd.print("G");
  lcd.write(0);
  lcd.print("s (0 a 100): ");
  Serial.println("Display LCD ligado"); // Informa, na porta serial, que o display esta ligado

  Serial.println("Iniciando cartão SD...");
  pinMode(CS, OUTPUT); // Configura o pino CS como saída
    
  if(SD.begin(CS)){ // Caso a inicialização do SD no pino CS ocorra com sucesso
    Serial.println("Cartão SD inicializado corretamente"); // Imprime uma mensagem de sucesso;
  }else{ // Caso a inicialização do SD no pino CS falhe
    Serial.println("Falha na inicialização do cartão SD"); // Imprime uma mensagem de erro
    while(1); // Inicia um loop sem fim pois o cartão não foi devidamente iniciado
  }
}

// Comandos que serao executados repetidamente (loop)
void loop(){

  gas = analogRead(pinoSensor); // Lê o pino analógico e armazena o valor recebido

  tempoColeta = millis(); // Armazena o tempo, em millisegundos, em que o dado foi coletado a partir do momento em que o Arduino é ligado
  tempoColeta /= 1000; // Converte esse valor de millisegundos para segundos
  
  gas = map(gas,0,1023,0,100); // Reinterpreta os dados obtidos, que variam de 0 a 1023 e passam a variar entre 0 e 100

  // Os proximos comandos imprimem a presença de gás na porta serial
  Serial.print("Presença de gás (escala de 0 a 100):"); 
  Serial.print(gas); // Imprime a presença de gás obtida

  // O resultado fica como:
  /*
  Presença de gás (escala de 0 a 100): 30 unidades
  */

  if(gas > gasLimiar){ // Caso seja detectada a presença de determinado gás, o seguinte bloco é excutado
  
    Serial.print(" - Gás detectado!"); // Imprime um aviso no monitor serial, indicando a detecção de certo gás
    
    if(LEDeBuzzerVariaveis == false){ // Caso o usuário deseja que o LED e o buzzer sejam estáticos
      
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

  lcd.backlight(); // Liga a luz de fundo do display

  // Os proximos comandos imprimem a presença de gás no display LCD
  lcd.setCursor(0, 1); // Posiciona o cursor na primeira coluna (0) e na segunda linha (1)
  lcd.print(gas); // Imprime o valor da prsença de gás coletada

  // O resultado fica como:
  /*
    Gás (0 a 100):
    56
  */

  delay(LCD_tempoAceso * 1000); // Estabelece um intervalo em milisegundos entre as medicoes

  // Apaga e reescreve o valor no display, fazendo o valor piscar
  for (int i = 0; i < numeroPiscadas; i++) {
    lcd.setCursor(0, 1);
    for (int j = 0; j < LCD_colunas; j++) {
      lcd.print(" ");
    }
    
    delay((LCD_tempoPiscando * 1000)/(numeroPiscadas * 2)); // Gere uma pausa entre os processos de apagar e reescreer os dados

    lcd.setCursor(0, 1); // Posiciona o cursor na primeira coluna (0) e na segunda linha (1)
    lcd.print(gas); // Imprime o valor da presença de gás coletada

    delay((LCD_tempoPiscando * 1000)/(numeroPiscadas * 2)); // Gere uma pausa entre os processos de apagar e reescreer os dados
  }

  lcd.noBacklight(); // Desliga a luz de fundo do display

  delay(LCD_tempoApagado*1000); // Estabelece um intervalo em milisegundos entre as medicoes

  // Abrir um arquivo no cartão SD e escrever nele
  File dataFile = SD.open("data.csv", FILE_WRITE); // Abre o arquivo data.csv e posiciona o cursor no seu fim
  
  if(dataFile){ // Caso o arquivo seja aberto com sucess  
    // Imprimir os dados no arquivo aberto
    dataFile.print(tempoColeta); // Imprime o tempo em milissegundos em que essa coleta foi realizada
    dataFile.print(","); // Imprime a vírgula para separar os dados
    //dataFile.print(gas); // Imprime a temperatura coletada
    dataFile.print("0");
    dataFile.println(); // Coloca uma quebra de linha
  
    // Fechar o arquivo e salvar as modificações
    dataFile.close();
  
  }else{ // Caso o arquivo nao possa ser aberto
  
    Serial.println("Erro ao tentar abrir data.csv");
  
  }
}
