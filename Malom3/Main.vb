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


Public Class Game
    Private _Ply(1) As Player 'players in the game
    Public history As New LinkedList(Of GameState) 'GameStates in this (and previous) games
    Private current As LinkedListNode(Of GameState) 'the node of the current GameState in history
    'Private MoveList As New List(Of Move)
    'Private MoveListCurIndex As Integer = 0
    Public ReadOnly Property s As GameState 'wrapper of current.value
        Get
            Return current.Value
        End Get
    End Property

    Public Sub New(ByVal p1 As Player, ByVal p2 As Player)
        history.AddLast(New GameState)
        current = history.Last
        Ply(0) = p1
        Ply(1) = p2
    End Sub

    Public Function Plys() As Player()
        Return _Ply
    End Function
    Public Property Ply(ByVal i As Integer) As Player 'get or set players in the game
        Get
            Return _Ply(i)
        End Get
        Set(ByVal p As Player)
            If p Is Nothing Then
                _Ply(i) = Nothing
                Return
            End If

            p.Quit() 'we exit p to see if it was in a game (e.g. NewGame in the previous one)
            If _Ply(i) IsNot Nothing Then _Ply(i).Quit() 'the player replaced by p is kicked out
            _Ply(i) = p
            p.Enter(Me)
        End Set
    End Property


    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Public Sub MakeMove(ByVal M As Move) 'called by player objects when they want to move
        Try
            'Debug.Print(Microsoft.VisualBasic.Timer & " MakeMove, sidetomove: " & s.SideToMove) '

            Ply(1 - s.SideToMove).FollowMove(M)

            history.AddAfter(current, New GameState(s))
            current = current.Next

            s.MakeMove(M)
        Catch ex As Exception
            If TypeOf ex Is KeyNotFoundException Then Throw
            MsgBox("Exception in MakeMove" & vbCrLf & ex.ToString)
        End Try
    End Sub

    Public Sub ApplySetup(toSet As GameState)
        history.AddAfter(current, toSet)
        current = current.Next
    End Sub

    Public Sub CancelThinking()
        For i = 0 To 1
            Ply(i).CancelThinking()
        Next
    End Sub
    Public Function PlayertypeChangingCmdAllowed() As Boolean
        'Return TypeOf Ply(s.SideToMove) Is HumanPlayer
        Return True
    End Function

    Public Sub CopyMoveList()
        Throw New NotImplementedException

        'this is buggy with undo

        'Dim s = ""
        'For i = 0 To MoveListCurIndex - 1
        '    s &= MoveList(i).ToString
        '    If i < MoveListCurIndex - 1 AndAlso Not TypeOf MoveList(i + 1) Is LeveszKorong Then s &= ", "
        'Next
        'Clipboard.SetText(s)
    End Sub
End Class

Public Class GameState
    Public T(23) As Integer 'the board (-1: empty, 0: white piece, 1: black piece)
    Public phase As Integer = 1
    Public SetStoneCount(1) As Integer 'how many stones the players have set
    Public StoneCount(1) As Integer
    Public KLE As Boolean 'is there a puck removal coming?
    Public SideToMove As Integer
    Public MoveCount As Integer
    Public over As Boolean
    Public winner As Integer '(-1, if a draw)
    Public block As Boolean
    Public LastIrrev As Integer

    Public Function FutureStoneCount(p As Integer) As Integer
        Return StoneCount(p) + MaxKSZ - SetStoneCount(p)
    End Function

    Public Sub New() 'start of game
        For i = 0 To 23
            T(i) = -1
        Next
    End Sub
    Public Sub New(ByVal s As GameState)
        T = s.T.ToArray 'deep copy 
        phase = s.phase
        SetStoneCount = s.SetStoneCount.ToArray
        StoneCount = s.StoneCount.ToArray
        KLE = s.KLE
        SideToMove = s.SideToMove
        MoveCount = s.MoveCount
        over = s.over
        winner = s.winner
        block = s.block
        LastIrrev = s.LastIrrev
    End Sub

    'Sets the state for Setup Mode: the placed stones are unchanged, but we switch to phase 2.
    Public Sub InitSetup()
        'T is unchanged
        'phase is unchanged
        'SetStoneCount is unchanged
        'StoneCount is unchanged, since T is unchanged
        'KLE is unchanged
        'SideToMove is unchanged
        MoveCount = 10 'majdnem mindegy, csak ne legyen tul kicsi, ld. mashol comment
        over = False
        'Winner can be undefined, as over = False
        block = False
        LastIrrev = 0
    End Sub

    Public Sub MakeMove(ByVal M As Object)
        If Not TypeOf (M) Is Move Then Throw New ArgumentException()

        CheckInvariants()
        CheckValidMove(M)

        MoveCount += 1
        If TypeOf M Is SetKorong Then
            T(M.hov) = SideToMove
            SetStoneCount(SideToMove) += 1
            StoneCount(SideToMove) += 1
            LastIrrev = 0
        ElseIf TypeOf M Is MoveKorong Then
            T(M.hon) = -1
            T(M.hov) = SideToMove
            LastIrrev += 1
            If LastIrrev >= LastIrrevLimit Then
                over = True
                winner = -1 'draw
            End If
        ElseIf TypeOf M Is LeveszKorong Then
            T(M.hon) = -1
            StoneCount(1 - SideToMove) -= 1
            KLE = False
            'If szakasz = 2 And KorongCount(1 - SideToMove) = 2 Then
            If StoneCount(1 - SideToMove) + MaxKSZ - SetStoneCount(1 - SideToMove) < 3 Then 'TODO: refactor to call to FutureStoneCount
                over = True
                winner = SideToMove
            End If
            LastIrrev = 0
        End If
        If (TypeOf M Is SetKorong Or TypeOf M Is MoveKorong) AndAlso Malome(M.hov, Me) > -1 And StoneCount(1 - SideToMove) > 0 Then 'if he made a monkey, your move is your opponent's puck
            KLE = True
        Else
            SideToMove = 1 - SideToMove
            If SetStoneCount(0) = MaxKSZ And SetStoneCount(1) = MaxKSZ And phase = 1 Then phase = 2 'switching to disc movement
            If Not YouCanMove(Me) Then
                over = True
                block = True
                winner = 1 - SideToMove
                If Wrappers.Constants.FBD AndAlso StoneCount(0) = 12 AndAlso StoneCount(1) = 12 Then
                    winner = -1
                End If
            End If
        End If

        CheckInvariants()
    End Sub

    Private Sub CheckValidMove(M As Move)
        Debug.Assert(Not over Or winner = -1) 'Nehez megcsinalni, hogy az 'over and winner = -1' eset sose forduljon elo. Pl. a PerfectPlayer.MakeMoveInState-nek a WithTaking esete gazos, mert lehet, hogy az elotte levo MakeMove mar dontetlenne tette
        If TypeOf M Is SetKorong Then
            Debug.Assert(phase = 1)
            Dim setKorong = CType(M, SetKorong)
            Debug.Assert(T(setKorong.hov) = -1)
        End If
        If TypeOf M Is MoveKorong Then
            Dim slide = CType(M, MoveKorong)
            Debug.Assert(T(slide.hon) = SideToMove)
            Debug.Assert(T(slide.hov) = -1)
        End If
        If TypeOf M Is LeveszKorong Then
            Debug.Assert(KLE)
            Dim take = CType(M, LeveszKorong)
            Debug.Assert(T(take.hon) = 1 - SideToMove)
        End If
    End Sub

    Private Sub CheckInvariants()
        Debug.Assert(SetStoneCount(0) >= 0)
        Debug.Assert(SetStoneCount(0) <= Rules.MaxKSZ)
        Debug.Assert(SetStoneCount(1) >= 0)
        Debug.Assert(SetStoneCount(1) <= Rules.MaxKSZ)
        Debug.Assert(phase = 1 Or phase = 2 And SetStoneCount(0) = MaxKSZ And SetStoneCount(1) = MaxKSZ)
    End Sub

    'Called when applying a free setup. It sets over and checks whether the position is valid. Returns "" if valid, reason str otherwise.
    'Also called when pasting a position.
    Public Function SetOverAndCheckValidSetup() As String
        Debug.Assert(Not over And Not block)

        'Validity checks:
        'Note: this should be before setting over, because we will deny applying the setup if the state is not valid, and we want to maintain the 'Not over and Not block' invariants.

        Dim toBePlaced0 = Rules.MaxKSZ - SetStoneCount(0)
        If StoneCount(0) + toBePlaced0 > Rules.MaxKSZ Then
            Return "Too many white stones (on the board + to be placed). Please remove some white stones from the board and/or decrease the number of white stones to be placed."
        End If
        Dim toBePlaced1 = Rules.MaxKSZ - SetStoneCount(1)
        If StoneCount(1) + toBePlaced1 > Rules.MaxKSZ Then
            Return "Too many black stones (on the board + to be placed). Please remove some black stones from the board and/or decrease the number of black stones to be placed."
        End If

        Debug.Assert(Not (phase = 1 And toBePlaced0 = 0 And toBePlaced1 = 0))
        Debug.Assert(Not (phase = 2 And (toBePlaced0 > 0 Or toBePlaced1 > 0)))
        If Wrappers.Constants.Variant <> Wrappers.Constants.Variants.lask And Not Wrappers.Constants.Extended Then
            If phase = 1 Then
                '(Amugy ezek a feltetelek kizarnak nehany olyan allast is, amihez van adatbazisunk: a szintukrozes miatt pl. van adatbazis olyan allashoz, ahol fekete kovetkezik, es egyenlo a toBePlaced)
                If toBePlaced0 <> toBePlaced1 - If(SideToMove = 0 Xor KLE, 0, 1) Then
                    Return "If Black is to move in the placement phase, then the number of black stones to be placed should be one more than the number of white stones to placed. If White is to move in the placement phase, then the number of white and black stones to be placed should be equal. (Except in a stone taking position, where these conditions are reversed.)" & vbCrLf & vbCrLf & "Note: The Lasker variant (and the extended solutions) doesn't have these constraints." & vbCrLf & vbCrLf & "Note: You can switch the side to move by the ""Switch STM"" button in position setup mode."
                End If
            Else
                Debug.Assert(phase = 2)
                Debug.Assert(toBePlaced0 = 0 And toBePlaced1 = 0)
            End If
        End If

        If KLE And StoneCount(1 - SideToMove) = 0 Then
            Return "A position where the opponent doesn't have any stones cannot be a stone taking position."
        End If

        '-----------------------------------------------------------
        'Set over if needed:

        Dim whiteLose, blackLose
        If StoneCount(0) + MaxKSZ - SetStoneCount(0) < 3 Then
            whiteLose = True
        End If
        If StoneCount(1) + MaxKSZ - SetStoneCount(1) < 3 Then
            blackLose = True
        End If
        If whiteLose Or blackLose Then
            over = True
            If whiteLose And blackLose Then
                winner = -1 'draw
            Else
                If whiteLose Then
                    winner = 1
                Else
                    Debug.Assert(blackLose)
                    winner = 0
                End If
            End If
        End If
        If Not KLE AndAlso Not YouCanMove(Me) Then 'YouCanMove doesn't handle the KLE case. However, we should always have a move in KLE, see the validity check above.
            over = True
            block = True
            winner = 1 - SideToMove
            If Wrappers.Constants.FBD AndAlso StoneCount(0) = 12 AndAlso StoneCount(1) = 12 Then
                winner = -1
            End If
        End If

        'Even though LastIrrev is always 0 while in free setup mode, it can be non-0 when pasting
        If LastIrrev >= Rules.LastIrrevLimit Then
            over = True
            winner = -1
        End If

        Return ""
    End Function


    Public Sub New(ByVal s As String) 'to paste from clipboard
        Dim ss() As String = s.Split(",")
        Try
            If ss(33) = "malom" OrElse ss(34) = "malom" OrElse ss(35) = "malom" OrElse ss(37) = "malom2" Then 'you need to be able to interpret older formats as well
                For i = 0 To 23
                    T(i) = ss(i)
                Next
                SideToMove = ss(24)
                phase = ss(27)
                SetStoneCount(0) = ss(28)
                SetStoneCount(1) = ss(29)
                StoneCount(0) = ss(30)
                StoneCount(1) = ss(31)
                KLE = ss(32)
                If ss(33) <> "malom" Then MoveCount = ss(33) Else MoveCount = 10 'It's 10 just so it wouldn't be 0, because then it wouldn't think about the next two steps, because it would think that the game is just beginning.
                If ss(33) <> "malom" AndAlso ss(34) <> "malom" Then LastIrrev = ss(34) Else LastIrrev = 0
                If StoneCount(0) <> T.Count(Function(x) x = 0) Or StoneCount(1) <> T.Count(Function(x) x = 1) Then Throw New InvalidGameStateException(" Number of stones is incorrect.")
            Else
                Throw New FormatException
            End If
        Catch ex As InvalidGameStateException
            Throw ex
        Catch ex As Exception
            Throw New FormatException
        End Try
    End Sub

    Public Overrides Function ToString() As String 'for clipboard
        Dim s As New System.IO.StringWriter
        For i = 0 To 23
            s.Write(T(i) & ",")
        Next
        s.Write(SideToMove & "," & 0 & "," & 0 & "," & phase & "," & SetStoneCount(0) & "," & SetStoneCount(1) & "," & StoneCount(0) & "," & StoneCount(1) & "," & KLE & "," & MoveCount & "," & LastIrrev)
        Return s.ToString()
    End Function
End Class

Class InvalidGameStateException
    Inherits Exception
    Public mymsg As String
    Public Sub New(ByVal msg As String)
        Me.mymsg = msg
    End Sub
End Class
