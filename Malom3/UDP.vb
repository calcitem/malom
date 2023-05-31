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


Module UDP
    Class UDPPlayer
        Inherits Player

        Private Port As Integer
        Const PortSt As Integer = 10000
        Private UdpThread As System.Threading.Thread

        Public Sub New()
            UdpThread = New System.Threading.Thread(AddressOf ProgMatch)
            UdpThread.IsBackground = True
            UdpThread.Start()
        End Sub

        Public Overrides Sub ToMove(ByVal s As GameState)

        End Sub
        Public Overrides Sub FollowMove(ByVal M As Object)
            If TypeOf M Is SetKorong Then Send("s " & M.GetMezok()(0))
            If TypeOf M Is LeveszKorong Then Send("l " & M.GetMezok()(0))
            If TypeOf M Is MoveKorong Then Send("m " & M.GetMezok()(0) & " " & M.GetMezok()(1))
        End Sub

        Public Overrides Sub Enter(ByVal G As Game)
            MyBase.Enter(G)
            If UdpThread.ThreadState = Threading.ThreadState.Running Then UdpThread.Resume()
        End Sub

        Public Overrides Sub Quit()
            If UdpThread IsNot Nothing AndAlso UdpThread.ThreadState = Threading.ThreadState.Running Then UdpThread.Suspend()
        End Sub

        Public Overrides Sub Over(ByVal s As GameState)
            If s.winner = -1 OrElse Not Object.ReferenceEquals(G.Ply(s.winner), Me) Then 'a nyertes írja be
                If Not s.winner = -1 OrElse Object.ReferenceEquals(G.Ply(1), Opponent) Then
                    System.IO.File.AppendAllText(My.Application.Info.DirectoryPath & "\match.txt", If(s.winner = -1, "döntetlen" & vbCrLf, Port & vbCrLf))
                End If
            End If
            'RecordResult(s)
        End Sub
        Private Sub Send(ByVal s As String)
            Dim cl As New System.Net.Sockets.UdpClient()
            Dim dg As Byte() = System.Text.Encoding.ASCII.GetBytes(s)
            cl.Send(dg, dg.Length, New System.Net.IPEndPoint(System.Net.IPAddress.Loopback, PortSt + 1 - (Port - PortSt)))
        End Sub

        Private Sub ProgMatch()
            System.IO.File.Delete(My.Application.Info.DirectoryPath & "\match.txt")
            Dim cl As System.Net.Sockets.UdpClient
            Port = PortSt
            Try
                cl = New System.Net.Sockets.UdpClient(Port)
            Catch ex As System.Net.Sockets.SocketException
                Port += 1
                cl = New System.Net.Sockets.UdpClient(Port)
            End Try
            Dim ep As New System.Net.IPEndPoint(System.Net.IPAddress.Loopback, 0)
            Do
                Dim Bytes As Byte() = cl.Receive(ep)
                Dim mv As String = System.Text.Encoding.ASCII.GetString(Bytes)
            Loop Until False
        End Sub
    End Class
End Module
