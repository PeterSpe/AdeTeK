Imports System
Imports System.Windows.Forms

Namespace AdeTeK_Plottertest
    Public Partial Class Form1
        Inherits Form

        Public Shared StepsProMM As Double = 40.0 'Bei 1600 Steps/Umdrehung

        'BEGIN Für die Motorfahrt
        Public Shared sollPosition_X As Integer
        Public Shared sollPosition_Y As Integer
        Public Shared istPosition_X As Integer = 0
        Public Shared istPosition_Y As Integer = 0
        Public Shared stepsToGo_X As Integer = 0
        Public Shared stepsToGo_Y As Integer = 0
        Public Shared X_Steps, Y_Steps, SummeSteps_XY As Integer
        Public Shared sleepPause As Integer
        'END Für die Motorfahrt

        '###########################################################
        Public Delegate Sub SetTextCallback(ByVal text As String)
        '###########################################################
        Public Sub New()
            Me.InitializeComponent()
        End Sub
        '###########################################################
        Private Sub ReceivedText(ByVal text As String)
            If Me.tbReceived.InvokeRequired Then
                Dim x = New SetTextCallback(AddressOf ReceivedText)
                Invoke(x, New Object() {text})
            Else
                Me.tbReceived.Text += text
            End If
        End Sub
        '###########################################################
        Private Sub serialPort1_DataReceived(ByVal sender As Object, ByVal e As Ports.SerialDataReceivedEventArgs)
            Me.ReceivedText(Me.serialPort1.ReadExisting())
        End Sub
        '###########################################################
        Private Sub Sleep(ByVal sleeptime As Integer)
            Dim stopw = New Stopwatch()
            stopw.Start()

            While stopw.ElapsedMilliseconds < sleeptime
                Call Application.DoEvents()
            End While

            stopw.Stop()
            stopw.Reset()
        End Sub

        Private Sub MsgBox(ByVal v As String)
            MsgBox("COM Port nicht gefunden...")
        End Sub
        '###########################################################
        Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs)
            Try
                Me.serialPort1.Open() '(Exception ex)
            Catch
                MsgBox("COM Port nicht gefunden...")
            End Try
            'timer1.Start();
            Me.panel1.Enabled = True
            Me.btnConnect.Enabled = False
            Me.btnDisconnect.Enabled = True
        End Sub
        '-----------------------------------------------------------
        Private Sub timer1_Tick(ByVal sender As Object, ByVal e As EventArgs)
            'try
            '{
            '    string Text = serialPort1.ReadExisting();
            '    tbReceived.Text += Text;
            '    tbxTest1.Text += Text;
            '}
            'catch //(Exception ex)
            '{
            '}           
        End Sub
        '-----------------------------------------------------------
        Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Close()
            Me.panel1.Enabled = False
            Me.btnConnect.Enabled = True
            Me.btnDisconnect.Enabled = False
        End Sub
        '-----------------------------------------------------------
        Private Sub BtnSend_Click(ByVal sender As Object, ByVal e As EventArgs)
            Try
                Me.serialPort1.Write(Me.tbTxtTransmit.Text) ' WriteLine und nicht nur Write!!!!!
                '(Exception ex)
            Catch
                MsgBox("COM Port nicht verbunden")
            End Try
        End Sub
        '-----------------------------------------------------------
        Private Sub btnExit_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Close()
            Close()
        End Sub
        '-----------------------------------------------------------
        Private Sub btnReset_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Write("2,0,0,0,0")      'Reset    case 2:
        End Sub
        'case 5: Arduino
        Private Sub btnServoUp_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Write("5,0,0,0,0")    'ServoUp    case 5:
        End Sub

        Private Sub btnServoDown_Click(ByVal sender As Object, ByVal e As EventArgs)
            'case 6: Arduino
            Me.serialPort1.Write("6,0,0,0,0")   'ServoDown   case 6:
        End Sub
        '-----------------------------------------------------------
        'case 7: Arduino
        Private Sub btnMagnetON_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Write("7,0,0,0,0")     'MagnetON case 7:
        End Sub
        '-----------------------------------------------------------
        'case 8: Arduino
        Private Sub btnMagnetOFF_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Write("8,0,0,0,0")    'MagnetOFF case 8:
        End Sub
        '-----------------------------------------------------------
        Private Sub btnPickUpFigure_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Write("6,0,0,0,0") 'ServoDown case 6:
            Sleep(2000)
            Me.serialPort1.Write("7,0,0,0,0") 'MagnetON  case 7:
            Sleep(2000)
            Me.serialPort1.Write("5,0,0,0,0") 'ServoUp   case 5:
        End Sub
        '-----------------------------------------------------------
        Private Sub btnPutDownFigure_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.serialPort1.Write("6,0,0,0,0") 'ServoDown case 6:
            Sleep(2000)
            Me.serialPort1.Write("8,0,0,0,0") 'MagnetOFF case 8:
            Sleep(2000)
            Me.serialPort1.Write("5,0,0,0,0") 'ServoUp   case 5:
        End Sub
        '-----------------------------------------------------------
        Private Sub btnTbReceivedClear_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.tbReceived.Clear()
            Me.tbxTest1.Clear()
            Me.tbxTest2.Clear()
            Me.tbxTest3.Clear()
            Me.tbxTest4.Clear()
        End Sub
        '-----------------------------------------------------------
        Private Sub btn00_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(0, 0)
        End Sub

        Private Sub btn1600_1600_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(1600, 1600)
        End Sub

        Private Sub btn6000_1000_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(6000, 1000)
        End Sub

        Private Sub btn3200_1600_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(3200, 1600)
        End Sub

        Private Sub btn2000_1000_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(2000, 1000)
        End Sub

        Private Sub btn4000_800_Click(ByVal sender As Object, ByVal e As EventArgs) 'geändert in 2000/4000
            FahrtXY(2000, 4000)
        End Sub

        Private Sub btn9900_700_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(9900, 700)
        End Sub

        Private Sub btnTextboxSend_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(5000, 3000)
        End Sub

        Private Sub btnLabelSend_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(2000, 5500)
        End Sub

        Private Sub Form1_Load(ByVal sender As Object, ByVal e As EventArgs)
        End Sub
        '-----------------------------------------------------------
        Private Sub btnTextBoxTest_Click(ByVal sender As Object, ByVal e As EventArgs)
            Me.tbReceived.Clear()
            Me.tbxTest1.Clear()
            Me.tbxTest2.Clear()
            Me.tbxTest3.Clear()
            Me.tbxTest4.Clear()
            Me.serialPort1.Write("14,0,0,0,0")
            Sleep(1500)
            Dim Zeile = Me.tbReceived.Lines

            Try
                Me.tbxTest1.Text = Zeile(0)
                Me.tbxTest2.Text = Zeile(1)
                Me.tbxTest3.Text = Zeile(2)
                Me.tbxTest4.Text = Zeile(3) '(Exception ex)
            Catch
                MsgBox("Es sind noch keine Werte vorhanden")
            End Try
        End Sub
        '-----------------------------------------------------------
        Private Sub btnTestVar_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(4650, 7280)
        End Sub
        '-----------------------------------------------------------
        Private Sub txbRunden1_MouseDoubleClick(ByVal sender As Object, ByVal e As MouseEventArgs)
            Me.serialPort1.Write("15," & Me.txbRunden1.Text & ",0,0,0")
        End Sub
        '-----------------------------------------------------------
        Private Sub btnStaticTime_Click(ByVal sender As Object, ByVal e As EventArgs)
            Dim Pausenzeit = 5000
            Me.tbxTest3.Text = "Pause: " & Pausenzeit.ToString()
            Me.serialPort1.Write("1,9000,8300,0,0")
            Sleep(Pausenzeit)
            Me.serialPort1.Write("1,0,0,0,0")
            Sleep(Pausenzeit)
            Me.serialPort1.Write("1,5000,7900,0,0")
            Sleep(Pausenzeit)
            Me.serialPort1.Write("1,0,0,0,0")
        End Sub
        '-----------------------------------------------------------
        Private Sub txbRunden1_KeyPress(ByVal sender As Object, ByVal e As KeyPressEventArgs)
            If e.KeyChar = 13 Then
                Me.serialPort1.Write("15," & Me.txbRunden1.Text & ",0,0,0")
                e.Handled = True 'DingDong ausschalten...
            End If
        End Sub

        Public Shared Function map(ByVal value As Integer, ByVal fromLow As Integer, ByVal fromHigh As Integer, ByVal toLow As Integer, ByVal toHigh As Integer) As Integer
            Return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow
        End Function 'END private static int map()------------------------------
        '-----------------------------------------------------------
        Private Sub btnWhileSchleife_Click(ByVal sender As Object, ByVal e As EventArgs)
            FahrtXY(9000, 8300)
            Sleep(sleepPause)
            FahrtXY(0, 0)
            Sleep(sleepPause)
            FahrtXY(5000, 7900)
            Sleep(sleepPause)
            FahrtXY(0, 0)
            Sleep(sleepPause)
            FahrtXY(1200, 300)
            Sleep(sleepPause)
            FahrtXY(0, 0)
            Sleep(sleepPause)
        End Sub
        '-----------------------------------------------------------
        Public Sub FahrtXY(ByVal Steps_X As Integer, ByVal Steps_Y As Integer)
            sollPosition_X = Steps_X
            sollPosition_Y = Steps_Y
            stepsToGo_X = Math.Abs(sollPosition_X - istPosition_X)
            stepsToGo_Y = Math.Abs(sollPosition_Y - istPosition_Y)
            SummeSteps_XY = stepsToGo_X + stepsToGo_Y
            Me.tbxSumSteps.Text = SummeSteps_XY.ToString()
            '#######################################################
            'this.serialPort1.Write("1,Steps_X, Steps_Y,0,0"); //--> So funktioniert es nicht...
            Me.serialPort1.Write("1," & Steps_X & "," & Steps_Y & ",0,0") 'zur seriellen Schnittstelle
            '#######################################################
            istPosition_X = sollPosition_X
            istPosition_Y = sollPosition_Y
            sleepPause = map(SummeSteps_XY, 500, 10000, 1000, 4500)
            'tbxSleepPause.Text = sleepPause.ToString();
            Me.tbxTest4.Text = "Pause: " & sleepPause.ToString()
        End Sub
        '-----------------------------------------------------------

    End Class 'END public partial class Form1 : Form
End Namespace 'END namespace AdeTeK_Plottertest

