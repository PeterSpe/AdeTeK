using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AdeTeK_Plottertest
{
    public partial class Form1 : Form
    {
        public static Double StepsProMM = 40.0; //Bei 1600 Steps/Umdrehung

        //BEGIN Für die Motorfahrt
        public static int sollPosition_X;
        public static int sollPosition_Y;
        public static int istPosition_X = 0;
        public static int istPosition_Y = 0;
        public static int stepsToGo_X = 0;
        public static int stepsToGo_Y = 0;
        public static int X_Steps, Y_Steps, SummeSteps_XY;
        public static int sleepPause;
        //END Für die Motorfahrt

        //###########################################################
        public delegate void SetTextCallback(string text);
        //###########################################################
        public Form1()
        {
            InitializeComponent();
        }
        //###########################################################
        private void ReceivedText(string text)
        {
            if (this.tbReceived.InvokeRequired)
            {
                var x = new SetTextCallback(ReceivedText);
                this.Invoke(x, new object[] { text });
            }
            else
            {
                this.tbReceived.Text += text;
            }
        }
        //###########################################################
        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            this.ReceivedText(this.serialPort1.ReadExisting());
        }
        //###########################################################
        private void Sleep(int sleeptime)
        {
            var stopw = new System.Diagnostics.Stopwatch();
            stopw.Start();
            while (stopw.ElapsedMilliseconds < sleeptime)
                Application.DoEvents();
            stopw.Stop();
            stopw.Reset();
        }
        private void MsgBox(string v)
        {
            MsgBox("COM Port nicht gefunden...");
        }
        //###########################################################
        private void btnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.Open();
            }
            catch //(Exception ex)
            {
                MsgBox("COM Port nicht gefunden...");
            }
            //timer1.Start();
            panel1.Enabled = true;
            btnConnect.Enabled = false;
            btnDisconnect.Enabled = true;
            serialPort1.Write("6,0,0,0,0"); //ServoDown case 6:
            Sleep(2000);
            serialPort1.Write("5,0,0,0,0"); //ServoUp   case 5:
            txbServoUpdatelAngleMin.Text = "10";
            txbServoUpdatelAngleMax.Text = "170";
        }          
        //-----------------------------------------------------------
        private void timer1_Tick(object sender, EventArgs e)
        {
            //try
            //{
            //    string Text = serialPort1.ReadExisting();
            //    tbReceived.Text += Text;
            //    tbxTest1.Text += Text;
            //}
            //catch //(Exception ex)
            //{
            //}           
        }
        //-----------------------------------------------------------
        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            serialPort1.Close();
            panel1.Enabled = false;
            btnConnect.Enabled = true;
            btnDisconnect.Enabled = false;
        }
        //-----------------------------------------------------------
        private void BtnSend_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.Write(tbTxtTransmit.Text); // WriteLine und nicht nur Write!!!!!
            }
            catch //(Exception ex)
            {
                MsgBox("COM Port nicht verbunden");
            }
        }
        //-----------------------------------------------------------
        private void btnExit_Click(object sender, EventArgs e)
        {
            serialPort1.Close();
            this.Close();
        }
        //-----------------------------------------------------------
        private void btnReset_Click(object sender, EventArgs e)
        {
            serialPort1.Write("2,0,0,0,0");      //Reset    case 2:
        }

        private void btnServoUp_Click(object sender, EventArgs e)
        //case 5: Arduino
        {
            serialPort1.Write("5,0,0,0,0");    //ServoUp    case 5:
        }

        private void btnServoDown_Click(object sender, EventArgs e)
        {
            //case 6: Arduino
            serialPort1.Write("6,0,0,0,0");   //ServoDown   case 6:

        }
        //-----------------------------------------------------------
        private void btnMagnetON_Click(object sender, EventArgs e)
        //case 7: Arduino
        {
            serialPort1.Write("7,0,0,0,0");     //MagnetON case 7:
        }
        //-----------------------------------------------------------
        private void btnMagnetOFF_Click(object sender, EventArgs e)
        //case 8: Arduino
        {
            serialPort1.Write("8,0,0,0,0");    //MagnetOFF case 8:
        }
        //-----------------------------------------------------------
        private void btnPickUpFigure_Click(object sender, EventArgs e)
        {
            serialPort1.Write("6,0,0,0,0"); //ServoDown case 6:
            Sleep(2000);
            serialPort1.Write("7,0,0,0,0"); //MagnetON  case 7:
            Sleep(2000);
            serialPort1.Write("5,0,0,0,0"); //ServoUp   case 5:
        }
        //-----------------------------------------------------------
        private void btnPutDownFigure_Click(object sender, EventArgs e)
        {
            serialPort1.Write("6,0,0,0,0"); //ServoDown case 6:
            Sleep(2000);
            serialPort1.Write("8,0,0,0,0"); //MagnetOFF case 8:
            Sleep(2000);
            serialPort1.Write("5,0,0,0,0"); //ServoUp   case 5:
        }
        //-----------------------------------------------------------
        private void btnTbReceivedClear_Click(object sender, EventArgs e)
        {
            tbReceived.Clear();
            tbxTest1.Clear();
            tbxTest2.Clear();
            tbxTest3.Clear();
            tbxTest4.Clear();
        }
        //-----------------------------------------------------------
        private void btn00_Click(object sender, EventArgs e)
        {
            FahrtXY(0, 0);
        }
        private void btn1600_1600_Click(object sender, EventArgs e)
        {

            FahrtXY(1600, 1600);
        }

        private void btn6000_1000_Click(object sender, EventArgs e)
        {
            FahrtXY(6000, 1000);
        }

        private void btn3200_1600_Click(object sender, EventArgs e)
        {
            FahrtXY(3200, 1600);
        }

        private void btn2000_1000_Click(object sender, EventArgs e)
        {
            FahrtXY(2000, 1000);
        }

        private void btn4000_800_Click(object sender, EventArgs e)//geändert in 2000/4000
        {
            FahrtXY(2000, 4000);
        }

        private void btn9900_700_Click(object sender, EventArgs e)
        {
            FahrtXY(9900, 700);
        }

        private void btnTextboxSend_Click(object sender, EventArgs e)
        {
            FahrtXY(5000, 3000);
        }
        private void btnLabelSend_Click(object sender, EventArgs e)
        {
            FahrtXY(2000, 5500);
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            
        }
        //-----------------------------------------------------------
        private void btnTextBoxTest_Click(object sender, EventArgs e)
        {
            tbReceived.Clear();
            tbxTest1.Clear();
            tbxTest2.Clear();
            tbxTest3.Clear();
            tbxTest4.Clear();
            serialPort1.Write("14,0,0,0,0");
            Sleep(1500);
            var Zeile = this.tbReceived.Lines;
            try
            {
                this.tbxTest1.Text = Zeile[0];
                this.tbxTest2.Text = Zeile[1];
                this.tbxTest3.Text = Zeile[2];
                this.tbxTest4.Text = Zeile[3];
            }
            catch //(Exception ex)
            {
                MsgBox("Es sind noch keine Werte vorhanden");
            }
        }
        //-----------------------------------------------------------
        private void btnTestVar_Click(object sender, EventArgs e)
        {
            FahrtXY(4650, 7280);
        }
        //-----------------------------------------------------------
        private void txbRunden1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            serialPort1.Write("15," + txbRunden1.Text + ",0,0,0");
        }
        //-----------------------------------------------------------
        private void btnStaticTime_Click(object sender, EventArgs e)
        {
            int Pausenzeit = 5000;
            tbxTest3.Text = "Pause: "+ Pausenzeit.ToString();
            serialPort1.Write("1,600,700,0,0");
            Sleep(Pausenzeit);
            serialPort1.Write("1,0,0,0,0");
            Sleep(Pausenzeit);
            serialPort1.Write("1,400,350,0,0");
            Sleep(Pausenzeit);
            serialPort1.Write("1,0,0,0,0");
        }
        //-----------------------------------------------------------
        private void txbRunden1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 13)
            {
                serialPort1.Write("15," + txbRunden1.Text + ",0,0,0");
                e.Handled = true;//DingDong ausschalten...
            }
        }
        public static int map(int value, int fromLow, int fromHigh, int toLow, int toHigh)
        {
            return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
        }//END private static int map()------------------------------

        private void btnReadCurrentAngles_Click(object sender, EventArgs e)
        {
            tbReceived.Clear();                      //ServoReadAngles()   case 16:
            serialPort1.Write("16,0,0,0,0");
            Sleep(1500);
            var Zeile = this.tbReceived.Lines;
            try
            {
                this.txbServoActualAngleMin.Text = Zeile[0];
                this.txbServoActualAngleMax.Text = Zeile[1];
            }
            catch //(Exception ex)
            {
                MsgBox("Es sind noch keine Werte vorhanden");
            }
            //Sleep(1000);
            tbReceived.Clear();
        }

        private void btnWriteDefaultAngles_Click(object sender, EventArgs e)
        {
            serialPort1.Write("17,10,170,0,0");    //ServoUpdateAngles()   case 17:
            txbServoUpdatelAngleMin.Text = "10";
            txbServoUpdatelAngleMax.Text= "170";            
        }

        private void btnWriteUpdateAngles_Click(object sender, EventArgs e)
        {
            this.serialPort1.Write("17," + txbServoUpdatelAngleMin.Text + "," + txbServoUpdatelAngleMax.Text + ",0,0");
        }

        //-----------------------------------------------------------
        private void btnWhileSchleife_Click(object sender, EventArgs e)
        {
            FahrtXY(1200, 1300);
            Sleep(sleepPause);
            Sleep(1000); //zusätzliche Hilfe, map-Funktion funktioniert nicht immer in FahrtXY()

            FahrtXY(0, 0);
            Sleep(sleepPause);
            Sleep(1000); //zusätzliche Hilfe, map-Funktion funktioniert nicht immer in FahrtXY()

            FahrtXY(1500, 600);
            Sleep(sleepPause);
            Sleep(1000); //zusätzliche Hilfe, map-Funktion funktioniert nicht immer in FahrtXY()

            FahrtXY(0, 0);
            Sleep(sleepPause);
            Sleep(1000); //zusätzliche Hilfe, map-Funktion funktioniert nicht immer in FahrtXY()

            FahrtXY(2300, 1700);
            Sleep(sleepPause);
            Sleep(2000); //zusätzliche Hilfe, map-Funktion funktioniert nicht immer in FahrtXY()

            FahrtXY(0, 0);
            Sleep(sleepPause);
        }
        //-----------------------------------------------------------
        public void FahrtXY(int Steps_X, int Steps_Y)
        {
            //die map-Funktion ist noch nicht ausgereift. Mehrere Motorläufe hintereinander
            //funktionieren nicht immer...

            sollPosition_X = Steps_X;
            sollPosition_Y = Steps_Y;
            stepsToGo_X = Math.Abs(sollPosition_X - istPosition_X);
            stepsToGo_Y = Math.Abs(sollPosition_Y - istPosition_Y);
            SummeSteps_XY = stepsToGo_X + stepsToGo_Y;
            tbxSumSteps.Text = SummeSteps_XY.ToString();
            //#######################################################
            //this.serialPort1.Write("1,Steps_X, Steps_Y,0,0"); //--> So funktioniert es nicht...
            this.serialPort1.Write("1," + Steps_X + "," + Steps_Y + ",0,0");//zur seriellen Schnittstelle
            //#######################################################
            istPosition_X = sollPosition_X;
            istPosition_Y = sollPosition_Y;

            //map-Funktion ##########################################
            sleepPause = map(SummeSteps_XY, 500, 10000, 1000, 4500);
            //#######################################################

            //tbxSleepPause.Text = sleepPause.ToString();
            tbxTest4.Text = "Pause: " + sleepPause.ToString();             
        }
        //-----------------------------------------------------------

    }//END public partial class Form1 : Form
}//END namespace AdeTeK_Plottertest
