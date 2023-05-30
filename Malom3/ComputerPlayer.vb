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


Imports System.Threading

Class ComputerPlayer
    Inherits Player

    Dim Settings As FrmSettings
    
    Protected E As Engine

    Public Overrides Sub Enter(ByVal _g As Game)
        MyBase.Enter(_g)
        E.InitEngine()
    End Sub

    Public Overrides Sub Quit()
        'ezek a blokkok csakis ebben a sorrendben lehetnek!

        If G Is Nothing Then Return

        MyBase.Quit()

        E.EndTh = True
        If E.ThinkThread IsNot Nothing AndAlso E.ThinkThread.ThreadState = ThreadState.Running Then E.ThinkThread.Join()
        E.EndTh = False
    End Sub

    Dim StopThinkTimer As Threading.Timer
    Public Overrides Sub ToMove(ByVal _s As GameState)
        'Debug.Print(Microsoft.VisualBasic.Timer & " ComputerPlayer ToMove") '
        E.StopOpptime()
        E.s = _s

        If StopThinkTimer IsNot Nothing Then StopThinkTimer.Change(Timeout.Infinite, 0) 'a régi timer-t kikapcsoljuk
        StopThinkTimer = New Threading.Timer(New TimerCallback(AddressOf StopThinking), Nothing, CLng(Settings.timelimit * 1000 * 10 + 200), 0) '8000

        E.ThinkThread = New System.Threading.Thread(AddressOf ThinkAndInvoke)
        E.ThinkThread.Name = "ThinkThread"
        E.ThinkThread.Start(Tuple.Create(_s, Settings.timelimit))
    End Sub

    Public Sub ThinkAndInvoke(s0_idolimit As Tuple(Of GameState, Double))
        Dim result = E.Think(s0_idolimit)
        InvokeUseThResult(result)
    End Sub


    Public Overrides Sub OppToMove(ByVal _s As GameState)
        E.StopOpptime()
        E.s = _s
        E.OppTime = True
        If StopThinkTimer IsNot Nothing Then StopThinkTimer.Change(Timeout.Infinite, 0) 'a régi timer-t kikapcsoljuk
        E.ThinkThread = New System.Threading.Thread(AddressOf E.OppTimeThink)
        E.ThinkThread.Start(Tuple.Create(_s, Settings.timelimit))
    End Sub



    Private Sub StopThinking()
        E.EndTh = True
    End Sub

    Public Overrides Sub CancelThinking()
        E.CancelThinking()
    End Sub

    Sub InvokeUseThResult(result As Engine.ThinkResult)
        If E.cancel Then Return
    End Sub

    Sub UseThResult(ThResult As Engine.ThinkResult)
        'Debug.Print(Microsoft.VisualBasic.Timer & " UseThResult") '

        If E.OppTime Then
            'OppTime = False
            Exit Sub
        End If
        If E.ThinkThread.IsAlive Then E.ThinkThread.Join()

        If G Is Nothing Then Return 'ha kileptettek kozben 

        Dim ThTime As Double = Math.Truncate((System.DateTime.Now - ThResult.st).TotalSeconds * 100) / 100


        Select Case ThResult.BestMove.flm
            Case 0
                G.MakeMove(New SetKorong(ThResult.BestMove.hová))
            Case 1
                G.MakeMove(New LeveszKorong(ThResult.BestMove.honnan))
            Case 2, 3
                G.MakeMove(New MoveKorong(ThResult.BestMove.honnan, ThResult.BestMove.hová))
        End Select
    End Sub


End Class

Class CombinedPlayer
    Inherits ComputerPlayer

    Public Overrides Sub OppToMove(_s As GameState)
        MyBase.OppToMove(_s)
        E.Advisor.OppToMove(_s)
    End Sub
End Class