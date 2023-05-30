' Malom, a Nine Men's Morris (and variants) player and solver program.
' Copyright(C) 2007-2016  Gabor E. Gevay, Gabor Danner
' 
' See our webpage (and the paper linked from there):
' http://compalg.inf.elte.hu/~ggevay/mills/index.php
' 
' 
' This program is free software: you can redistribute it and/or modify
' it under the terms of the GNU General Public License as published by
' the Free Software Foundation, either version 3 of the License, or
' (at your option) any later version.
' 
' This program is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU General Public License for more details.
' 
' You should have received a copy of the GNU General Public License
' along with this program.  If not, see <http://www.gnu.org/licenses/>.


Imports System.Drawing
Imports System.Windows.Forms

Public Class Board
    Inherits PictureBox
    Private s As GameState
    Private offScrBmp As New Bitmap(Screen.PrimaryScreen.WorkingArea.Width, Screen.PrimaryScreen.WorkingArea.Height)
    Private g As Graphics = Graphics.FromImage(offScrBmp) 'ezzel rajzolunk
    Private tg As Graphics 'ezzel tesszük ki a látható felületre
    Private SelectedMezo As Integer = -1
    Private JelöltMezok() As Integer = {}
    Private LastJeloltMezok() As Integer = {}
    Private WithEvents MutatTimer As New Timer
    Public Advisor As PerfectPlayer

    Private Function MelyikMezo(ByVal x As Integer, ByVal y As Integer) As Integer 'megadja, hogy egy pont melyik mezõhöz tartozik (100-at ad, ha semelyikhez)
        Dim k As Rectangle
        MelyikMezo = 100
        For i = 0 To 23
            k.Height = (Me.Height + Me.Width) / 2 / 20
            k.Width = k.Height
            k.X = BoardNodes(i).X - k.Width / 2
            k.Y = BoardNodes(i).Y - k.Height / 2
            Dim p As New Point(x, y)
            If (k.Contains(p)) Then
                Return i
            End If
        Next
    End Function

    Private Sub SelectMezo(ByVal m As Integer)
        If m = SelectedMezo Then SelectedMezo = -1 Else SelectedMezo = m
        UpdateGameState()
    End Sub
    Public Sub ClearMezoSelection()
        SelectedMezo = -1
    End Sub

    Public Sub JelolMezo(ByVal m As Integer()) 'pöttyös jelölés
        JelöltMezok = m
        If m.Length > 0 Then
            MutatTimer.Stop()
            MutatTimer.Start()
            LastJeloltMezok = JelöltMezok.ToArray() 'deep copy
        End If
    End Sub
    Private Sub ClearJeloltMezok() 'eltünteti a pöttyös jelölést
        JelolMezo(New Integer() {})
        UpdateGameState()
        MutatTimer.Stop()
    End Sub
    Private Sub MutatTimer_Tick(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MutatTimer.Tick
        ClearJeloltMezok()
    End Sub
    Public Sub ShowLastJeloltMezok()
        JelolMezo(LastJeloltMezok)
        UpdateGameState() 'ez azért csak ide kell, mert normál esetben a MakeMove meghívja az UpdateUI-t
    End Sub

    Public Sub New()
    End Sub
    Private TáblaVonalSzín As Color = Color.Blue
    Private Function TáblaVonal() As Pen
        Return New Pen(TáblaVonalSzín, System.Math.Round((Me.Height + Me.Width) / (2304 / 10)))
    End Function
    Public Sub UpdateGameState(ByVal _s As GameState) 'átállítja a state-et, és rendereli
        s = _s
    End Sub
    Public Sub UpdateGameState()
        UpdateGameState(s)
    End Sub
    Public BoardNodes(23) As Point 'a mezõk pozíciói
    Private Sub DrawBoard()
        Dim p As Pen = TáblaVonal()
        Dim r As New Rectangle
        g.SmoothingMode = Drawing2D.SmoothingMode.Default

        'Dim kp(3), vp(3) As Point
        Dim OuterSquareRatio = 9.6 / 12
        r.Width = Me.Width * OuterSquareRatio
        r.Height = Me.Height * OuterSquareRatio
        r.X = Me.Width * (1 - (OuterSquareRatio)) / 2
        r.Y = Me.Height * (1 - (OuterSquareRatio)) / 2

        'kp(0).X = r.X
        'kp(0).Y = r.Y + r.Height / 2
        'kp(1).X = r.X + r.Width / 2
        'kp(1).Y = r.Y
        'kp(2).X = r.X + r.Width
        'kp(2).Y = r.Y + r.Height / 2
        'kp(3).X = r.X + r.Width / 2
        'kp(3).Y = r.Y + r.Height
        'g.DrawRectangle(p, r)

        BoardNodes(0).X = r.X
        BoardNodes(0).Y = r.Y + r.Height / 2
        BoardNodes(1).X = r.X
        BoardNodes(1).Y = r.Y
        BoardNodes(2).X = r.X + r.Width / 2
        BoardNodes(2).Y = r.Y
        BoardNodes(3).X = r.X + r.Width
        BoardNodes(3).Y = r.Y
        BoardNodes(4).X = r.X + r.Width
        BoardNodes(4).Y = r.Y + r.Height / 2
        BoardNodes(5).X = r.X + r.Width
        BoardNodes(5).Y = r.Y + r.Height
        BoardNodes(6).X = r.X + r.Width / 2
        BoardNodes(6).Y = r.Y + r.Height
        BoardNodes(7).X = r.X
        BoardNodes(7).Y = r.Y + r.Height

        Dim MiddleSquareRatio = 6.6 / 12
        r.Width = Me.Width * MiddleSquareRatio
        r.Height = Me.Height * MiddleSquareRatio
        r.X = Me.Width * (1 - (MiddleSquareRatio)) / 2
        r.Y = Me.Height * (1 - (MiddleSquareRatio)) / 2
        'g.DrawRectangle(p, r)

        BoardNodes(8).X = r.X
        BoardNodes(8).Y = r.Y + r.Height / 2
        BoardNodes(9).X = r.X
        BoardNodes(9).Y = r.Y
        BoardNodes(10).X = r.X + r.Width / 2
        BoardNodes(10).Y = r.Y
        BoardNodes(11).X = r.X + r.Width
        BoardNodes(11).Y = r.Y
        BoardNodes(12).X = r.X + r.Width
        BoardNodes(12).Y = r.Y + r.Height / 2
        BoardNodes(13).X = r.X + r.Width
        BoardNodes(13).Y = r.Y + r.Height
        BoardNodes(14).X = r.X + r.Width / 2
        BoardNodes(14).Y = r.Y + r.Height
        BoardNodes(15).X = r.X
        BoardNodes(15).Y = r.Y + r.Height

        Dim InnerSquareRatio = 3.6 / 12
        r.Width = Me.Width * InnerSquareRatio
        r.Height = Me.Height * InnerSquareRatio
        r.X = Me.Width * (1 - (InnerSquareRatio)) / 2
        r.Y = Me.Height * (1 - (InnerSquareRatio)) / 2
        'vp(0).X = r.X
        'vp(0).Y = kp(0).Y '
        'vp(1).X = kp(1).X '

        'vp(1).Y = r.Y
        'vp(2).X = r.X + r.Width
        'vp(2).Y = kp(2).Y '
        'vp(3).X = kp(3).X '
        'vp(3).Y = r.Y + r.Height
        'g.DrawRectangle(p, r)
        'For i = 0 To 3
        '    g.DrawLine(p, kp(i), vp(i))
        'Next

        BoardNodes(16).X = r.X
        BoardNodes(16).Y = r.Y + r.Height / 2
        BoardNodes(17).X = r.X
        BoardNodes(17).Y = r.Y
        BoardNodes(18).X = r.X + r.Width / 2
        BoardNodes(18).Y = r.Y
        BoardNodes(19).X = r.X + r.Width
        BoardNodes(19).Y = r.Y
        BoardNodes(20).X = r.X + r.Width
        BoardNodes(20).Y = r.Y + r.Height / 2
        BoardNodes(21).X = r.X + r.Width
        BoardNodes(21).Y = r.Y + r.Height
        BoardNodes(22).X = r.X + r.Width / 2
        BoardNodes(22).Y = r.Y + r.Height
        BoardNodes(23).X = r.X
        BoardNodes(23).Y = r.Y + r.Height

        'illesztés
        BoardNodes(8).Y = BoardNodes(0).Y
        BoardNodes(16).Y = BoardNodes(0).Y
        BoardNodes(10).X = BoardNodes(2).X
        BoardNodes(18).X = BoardNodes(2).X
        BoardNodes(12).Y = BoardNodes(4).Y
        BoardNodes(20).Y = BoardNodes(4).Y
        BoardNodes(14).X = BoardNodes(6).X
        BoardNodes(22).X = BoardNodes(6).X


        Dim j As Byte
        For i = 0 To 23
            For j = 1 To ALBoardGraph(i, 0)
                g.DrawLine(p, BoardNodes(i), BoardNodes(ALBoardGraph(i, j)))
            Next
        Next
    End Sub
    Private Sub DrawMezok()
        Dim AdvisorMoves As List(Of Tuple(Of PerfectPlayer.Move, Wrappers.gui_eval_elem2))
        Dim OkMoveTargets As SortedSet(Of Integer)
        Try
            If Advisor IsNot Nothing And Not s.over Then
                AdvisorMoves = Advisor.GetMoveList(s).Select(Function(m) Tuple.Create(m, Advisor.MoveValue(s, m))).ToList
                Dim OkMoves = PerfectPlayer.AllMaxBy(Function(mvp) mvp.Item2, AdvisorMoves, Wrappers.gui_eval_elem2.min_value(Advisor.GetSec(s))).Select(Function(mvp) mvp.Item1).ToList
                OkMoveTargets = New SortedSet(Of Integer)(OkMoves.Select(Function(m) If(s.KLE, m.TakeHon, m.hov)))

                For Each m In OkMoves
                    If m.MoveType = PerfectPlayer.MoveType.SlideMove Then
                        Dim pen = TáblaVonal()
                        pen.Color = Color.Yellow
                        g.DrawLine(pen, BoardNodes(m.hon), BoardNodes(m.hov))
                    End If
                Next
            End If
        Catch ex As KeyNotFoundException 'ez a no secfile esete
        End Try

        g.SmoothingMode = Drawing2D.SmoothingMode.AntiAlias
        For i = 0 To 23
            DrawMezo(i, AdvisorMoves, OkMoveTargets)
        Next
    End Sub
    Private Sub DrawMezo(ByVal i As Integer, AdvisorMoves As List(Of Tuple(Of PerfectPlayer.Move, Wrappers.gui_eval_elem2)), OkMoveTargets As SortedSet(Of Integer))
        g.SmoothingMode = Drawing2D.SmoothingMode.AntiAlias
        Dim AktKorong As RectangleF
        Dim b As New SolidBrush(Color.White)
        If s.T(i) = 1 Then b.Color = Color.Black
        AktKorong.Height = CSng(Me.Height + Me.Width) / 2 / 20
        AktKorong.Width = AktKorong.Height 'PBTábla.Width / 20
        AktKorong.X = BoardNodes(i).X - AktKorong.Width / 2
        AktKorong.Y = BoardNodes(i).Y - AktKorong.Height / 2
        If s.T(i) > -1 Then 'ha itt van korong
            g.FillEllipse(b, AktKorong)
            If i = SelectedMezo Then
                Dim p As New Pen(Color.Red) With {.Width = 1}
                g.DrawEllipse(p, AktKorong)
            Else
                Dim p As New Pen(Color.White) With {.Width = 1}
                If s.T(i) = 1 Then p.Color = Color.Black
                g.DrawEllipse(p, AktKorong)
            End If
            If OkMoveTargets IsNot Nothing AndAlso OkMoveTargets.Contains(i) Then 'Advisor for taking
                b.Color = Color.Yellow
                Const a As Single = 0.5
                AktKorong.X += AktKorong.Width * (1 - a) / 2
                AktKorong.Y += AktKorong.Height * (1 - a) / 2
                AktKorong.Height *= a
                AktKorong.Width *= a
                g.FillEllipse(b, AktKorong)
            End If
        Else
            If OkMoveTargets IsNot Nothing AndAlso OkMoveTargets.Contains(i) Then
                b.Color = Color.Yellow
            Else
                b.Color = TáblaVonalSzín
            End If
            Const a As Single = 0.6 'ennyiszerese az üres mezõ körének átmérõje a korongok átmérõjének
            AktKorong.X += AktKorong.Width * (1 - a) / 2
            AktKorong.Y += AktKorong.Height * (1 - a) / 2
            AktKorong.Height *= a
            AktKorong.Width *= a
            g.FillEllipse(b, AktKorong)
        End If

        If AdvisorMoves IsNot Nothing AndAlso AdvisorMoves.Any(Function(mvp) If(s.KLE, mvp.Item1.TakeHon, mvp.Item1.hov) = i) Then

            Dim old = Wrappers.gui_eval_elem2.ignore_DD
            Wrappers.gui_eval_elem2.ignore_DD = False
            Dim str = AdvisorMoves.Where(Function(mvp) If(s.KLE, mvp.Item1.TakeHon, mvp.Item1.hov) = i).Max(Function(mvp) mvp.Item2).ToString().Replace("(", vbCrLf + "(")
            Wrappers.gui_eval_elem2.ignore_DD = old

            Dim brush = If(str(0) = "W", Brushes.Brown, If(str(0) = "L", Brushes.Red, Brushes.Green))
            Dim offs = 24 * Me.Width / 1000
            g.DrawString(str, New Font("Arial", CSng(12 * Me.Width / 1000), FontStyle.Bold), brush, New Point(AktKorong.X + offs, AktKorong.Y + offs))
        End If
    End Sub

End Class