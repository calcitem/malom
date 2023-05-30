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


Imports System.IO
Imports Wrappers

Public Class Sectors
    Public Shared sectors As New Dictionary(Of id, Sector)
    Shared created As Boolean = False

    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Public Shared Function getsectors() As Dictionary(Of id, Sector)
        Try

            If Not created Then

                Wrappers.Init.init_sym_lookuptables()
                Wrappers.Init.init_sec_vals()

                For w = 0 To Rules.MaxKSZ
                    For b = 0 To Rules.MaxKSZ
                        For wf = 0 To Rules.MaxKSZ
                            For bf = 0 To Rules.MaxKSZ
                                Dim fname = String.Format(Rules.VariantName & "_{0}_{1}_{2}_{3}.sec" & Constants.Fname_suffix, w, b, wf, bf)
                                'Console.WriteLine("Looking for database file " & fname)
                                Dim id As New id(w, b, wf, bf)
                                If File.Exists(fname) Then
                                    sectors(id) = New Sector(id)
                                End If
                            Next
                        Next
                    Next
                Next

                created = True
            End If

            Return sectors

        Catch ex As Exception
            If TypeOf ex Is KeyNotFoundException Then Throw
            MsgBox("An error happened in getsectors" & vbCrLf & ex.ToString, MsgBoxStyle.Critical)
            Environment.Exit(1)
            Return Nothing
        End Try
    End Function

    Public Shared Function HasDatabase() As Boolean
        Return getsectors().Count > 0
    End Function
End Class

Public Class PerfectPlayer
    Inherits Player

    Dim secs As New Dictionary(Of id, Sector)
    Const UseWRGM = False 'Ha ez True, akkor az engine-ben korlatozni kell a melyseget (ez mostmar automatikus)
    Dim Eng As Engine

    Public Sub New()
        Debug.Assert(Sectors.HasDatabase)
        secs = Sectors.getsectors()

        If UseWRGM Then
            Debug.Assert(Rules.AlphaBetaAvailable)
            Eng = New Engine(Nothing, Nothing, Nothing, True)
            Eng.InitEngine()
        End If
    End Sub

    Public Overrides Sub Enter(ByVal _g As Game)
        MyBase.Enter(_g)
    End Sub

    Public Overrides Sub Quit()
        MyBase.Quit()
    End Sub

    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Public Function GetSec(s As GameState) As Sector
        Try
            If s.KLE Then Return Nothing

            Dim id As New id(s.StoneCount(0), s.StoneCount(1), Rules.MaxKSZ - s.SetStoneCount(0), Rules.MaxKSZ - s.SetStoneCount(1))

            If s.SideToMove = 1 Then
                id.negate()
            End If

            Return secs(id)
        Catch ex As Exception
            If TypeOf ex Is KeyNotFoundException Then Throw
            MsgBox("An error happened in GetSec" & vbCrLf & ex.ToString, MsgBoxStyle.Critical)
            Environment.Exit(1)
            Return Nothing
        End Try
    End Function

    Private Function NegateState(s As GameState) As GameState
        Dim r As New GameState(s)
        For i = 0 To 23
            If r.T(i) = 0 Then
                r.T(i) = 1
            ElseIf r.T(i) = 1 Then
                r.T(i) = 0
            End If
        Next
        r.StoneCount(0) = s.StoneCount(1)
        r.StoneCount(1) = s.StoneCount(0)
        r.SetStoneCount(0) = s.SetStoneCount(1)
        r.SetStoneCount(1) = s.SetStoneCount(0)
        Return r
    End Function

    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Public Shared Function ToHumanReadableEval(e As gui_eval_elem2) As String
        Try
            Return e.ToString()
        Catch ex As Exception
            MsgBox("An error happened in ToHumanReadableEval" & vbCrLf & ex.ToString, MsgBoxStyle.Critical)
            Environment.Exit(1)
            Return Nothing
        End Try
    End Function



    Public Enum MoveType
        SetMove
        SlideMove 'should be renamed to SlideOrJumpMove
    End Enum

    Structure Move
        Public hon, hov As Integer
        Public MoveType As MoveType
        Public WithTaking, OnlyTaking As Boolean 'A With-esben benne van a malombecsukas lepese is, az Only-sban pedig csak a levetel
        Public TakeHon As Integer

        Public Function ToBitBoard() As Integer
            If OnlyTaking Then
                Return 1 << TakeHon
            End If
            Dim ret As Integer = 1 << hov
            If MoveType = MoveType.SlideMove Then
                ret += 1 << hon
            End If
            If WithTaking Then
                ret += 1 << TakeHon
            End If
            Return ret
        End Function
    End Structure

    Private Function FutureKorongCount(s As GameState) As Integer
        Return s.StoneCount(s.SideToMove) + Rules.MaxKSZ - s.SetStoneCount(s.SideToMove) 'TODO: refactor to call to FutureStoneCount
    End Function

    Private Function MakesMill(ByVal s As GameState, ByVal hon As Integer, ByVal hov As Integer) As Boolean
        Dim s2 As New GameState(s)
        If hon <> -1 Then s2.T(hon) = -1
        s2.T(hov) = s.SideToMove
        Return -1 <> Rules.Malome(hov, s2)
    End Function

    Private Function IsMill(ByVal s As GameState, ByVal m As Integer) As Boolean
        Return -1 <> Rules.Malome(m, s)
    End Function

    Private Function SetMoves(ByVal s As GameState) As List(Of Move)
        Dim r As New List(Of Move)
        For i = 0 To 23
            If s.T(i) = -1 Then
                r.Add(New Move With {.hov = i, .WithTaking = MakesMill(s, -1, i), .MoveType = MoveType.SetMove})
            End If
        Next
        Return r
    End Function
    Private Function SlideMoves(ByVal s As GameState) As List(Of Move)
        Dim r As New List(Of Move)
        For i = 0 To 23
            For j = 0 To 23
                If s.T(i) = s.SideToMove And s.T(j) = -1 And (FutureKorongCount(s) = 3 Or Rules.BoardGraph(i, j)) Then
                    r.Add(New Move With {.hon = i, .hov = j, .WithTaking = MakesMill(s, i, j), .MoveType = MoveType.SlideMove})
                End If
            Next
        Next
        Return r
    End Function

    'm-ben egy WithTakinges lepes van, aminek nincsen kitoltve a TakeHonja. Ez a fuggveny keszit egy listat, aminek az elemei m masolatai, kiegeszitve egy-egy lehetseges levetellel
    Private Function WithTakingMoves(ByVal s As GameState, ByVal m As Move) As List(Of Move)
        Dim r As New List(Of Move)

        Dim EverythingInMill As Boolean = True
        For i = 0 To 23
            If s.T(i) = 1 - s.SideToMove And Not IsMill(s, i) Then EverythingInMill = False
        Next

        For i = 0 To 23
            If s.T(i) = 1 - s.SideToMove And (Not IsMill(s, i) Or EverythingInMill) Then
                Dim m2 = m
                m2.TakeHon = i
                r.Add(m2)
            End If
        Next

        Return r
    End Function
    Private Function OnlyTakingMoves(ByVal s As GameState) As List(Of Move) 'itt van nemi copy-paste kod
        Dim r As New List(Of Move)

        Dim EverythingInMill As Boolean = True
        For i = 0 To 23
            If s.T(i) = 1 - s.SideToMove And Not IsMill(s, i) Then EverythingInMill = False
        Next

        For i = 0 To 23
            If s.T(i) = 1 - s.SideToMove And (Not IsMill(s, i) Or EverythingInMill) Then
                r.Add(New Move With {.OnlyTaking = True, .TakeHon = i})
            End If
        Next

        Return r
    End Function


    Public Function GetMoveList(ByVal s As GameState) As List(Of Move)
        Dim ms0, ms As List(Of Move)
        If Not s.KLE Then
            If Wrappers.Constants.Variant = Wrappers.Constants.Variants.std Or Wrappers.Constants.Variant = Wrappers.Constants.Variants.mora Then
                If s.SetStoneCount(s.SideToMove) < Rules.MaxKSZ Then
                    ms0 = SetMoves(s)
                Else
                    ms0 = SlideMoves(s)
                End If
            Else 'Lasker
                ms0 = SlideMoves(s)
                If s.SetStoneCount(s.SideToMove) < Rules.MaxKSZ Then
                    ms0.AddRange(SetMoves(s))
                End If
            End If

            ms = New List(Of Move)
            For i = 0 To ms0.Count - 1
                If Not ms0(i).WithTaking Then
                    ms.Add(ms0(i))
                Else
                    ms.AddRange(WithTakingMoves(s, ms0(i)))
                End If
            Next
        Else 'KLE
            ms = OnlyTakingMoves(s)
        End If

        Return ms
    End Function

    Private Function MakeMoveInState(ByVal s As GameState, ByVal m As Move) As GameState
        Dim s2 As New GameState(s)
        If Not m.OnlyTaking Then
            If m.MoveType = MoveType.SetMove Then
                s2.MakeMove(New SetKorong(m.hov))
            Else
                s2.MakeMove(New MoveKorong(m.hon, m.hov))
            End If
            If m.WithTaking Then s2.MakeMove(New LeveszKorong(m.TakeHon))
        Else
            s2.MakeMove(New LeveszKorong(m.TakeHon))
        End If
        Return s2
    End Function

    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Public Function MoveValue(ByVal s As GameState, ByVal m As Move) As gui_eval_elem2
        Try
            Return Eval(MakeMoveInState(s, m)).undo_negate(GetSec(s))
        Catch ex As Exception
            If TypeOf ex Is KeyNotFoundException Then Throw
            MsgBox("Exception in MoveValue" & vbCrLf & ex.ToString, MsgBoxStyle.Critical)
            Environment.Exit(1)
        End Try
    End Function

    'Returns all elements of l, that are maximal by f
    Public Shared Function AllMaxBy(Of T, K As IComparable(Of K))(ByVal f As Func(Of T, K), ByVal l As List(Of T), MinValue As K) As List(Of T)
        Dim r As New List(Of T)
        'Dim ma = Integer.MinValue
        'Dim ma As K = f(l(0))
        Dim ma = MinValue
        For Each m In l
            Dim e = f(m)
            'If e > ma Then
            If e.CompareTo(ma) > 0 Then
                ma = e
                r.Clear()
                r.Add(m)
            Else
                If e.CompareTo(ma) = 0 Then
                    r.Add(m)
                End If
            End If
        Next
        Return r
    End Function

    Public Function GoodMoves(ByVal s As GameState) As List(Of Move)
        Return AllMaxBy(Function(m) MoveValue(s, m), GetMoveList(s), gui_eval_elem2.min_value(GetSec(s)))
    End Function

    Private Function NGMAfterMove(ByVal s As GameState, ByVal m As Move) As Integer
        Return NumGoodMoves(MakeMoveInState(s, m))
    End Function

    Private Function MinNGMMoves(ByVal s As GameState) As List(Of Move)
        Return AllMaxBy(Function(m) -NGMAfterMove(s, m), GoodMoves(s), Integer.MinValue)
    End Function

    Private Shared rnd As New Random()
    Public Function ChooseRandom(Of T)(ByVal l As List(Of T)) As T
        Return l(rnd.Next(l.Count))
    End Function

    Private Sub SendMoveToGUI(m As Move)
        If Not m.OnlyTaking Then
            If m.MoveType = MoveType.SetMove Then
                G.MakeMove(New SetKorong(m.hov))
            Else
                G.MakeMove(New MoveKorong(m.hon, m.hov))
            End If
        Else
            G.MakeMove(New LeveszKorong(m.TakeHon))
        End If
    End Sub

    'A koronglevetel kezelese:
    '-Az eval nem tud KLE allast kezelni, mivel a solver szempontjabol ezek nem leteznek.
    '-Tehat az itteni move strukturaban ossze van vonva a malombecsukas a koronglevetelevel.
    '-Viszont a GUI szempontjabol ez kulon esemeny.
    '-Vegulis az lett, hogy amikor a GUI-tol KLE allast kapunk, akkor ujra belenezunk az adatbazisba.
    '   (Igy tudjuk kezelni, ha pl. bepaste-elnek KLE allast (valamint jobban illeszkedik a regi engine-hez))
    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Public Overrides Sub ToMove(ByVal s As GameState)
        Try
            'Dim mh = GoodMoves(s)(0)
            'Dim mh = ChooseRandom(GoodMoves(s))
            'Dim mh = MinNGMMoves(s)(0)
            'Dim mh = RecWRGM(s).m
            'Dim mh = If(UseWRGM, ChooseRandom(MinWRGMMoves(s)), ChooseRandom(MinNGMMoves(s)))
            'Dim mh = MinNGMMoves(s)(0)

            'Dim mh As Move
            'If s.LépésCount > 6 Then
            '    mh = MinNGMMoves(s)(0)
            'Else
            '    mh = ChooseNonNTREKSMoveIfPossible(s)
            'End If

            'Dim mh = ChooseNonNTREKS8998MoveIfPossible(s)


            Dim mh = ChooseRandom(GoodMoves(s))
            'Dim mh = ChooseRandom(MinNGMMoves(s))


            'Dim ma = MoveValue(s, mh)

            'Main.LblPerfEval.Text = "Ev: " & ma

            SendMoveToGUI(mh)
        Catch ex As KeyNotFoundException
            'Debug.Assert(Not s.KLE) 'kivettuk, de nem tudjuk, hogy miert volt bent
            SendMoveToGUI(ChooseRandom(GetMoveList(s)))
        Catch ex As Exception
            MsgBox("Exception in ToMove" & vbCrLf & ex.ToString, MsgBoxStyle.Critical)
            Environment.Exit(1)
        End Try
    End Sub


    Private Function NumGoodMoves(ByVal s As GameState) As Integer
        If FutureKorongCount(s) < 3 Then Return 0
        Dim ma = gui_eval_elem2.min_value(GetSec(s)), mh As Move, c = 0
        For Each m In GetMoveList(s)
            Dim e = MoveValue(s, m)
            If e > ma Then
                ma = e
                mh = m
                c = 1
            Else
                If e = ma Then c += 1
            End If
        Next
        Return c
    End Function

    Dim cp As Integer
    Structure MoveValuePair
        Dim m As Move
        Dim val As Double
    End Structure
    Const WRGMInf As Double = 2 'jo ez?

    Private Shared EvalLock As New Object
    <System.Runtime.ExceptionServices.HandleProcessCorruptedStateExceptionsAttribute>
    Private Function Eval(ByVal s As GameState) As gui_eval_elem2
        Try
            SyncLock EvalLock 'ez azert kell, mert ha mindket jatekos hivogatja (az egyik esetleg OppTime), akkor a hash objektumokkal gond lehetne a dinamikus cserelgetes miatt
                Debug.Assert(Not s.KLE)
                Dim id As New id(s.StoneCount(0), s.StoneCount(1), Rules.MaxKSZ - s.SetStoneCount(0), Rules.MaxKSZ - s.SetStoneCount(1))

                If FutureKorongCount(s) < 3 Then Return gui_eval_elem2.virt_loss_val

                Dim a As Int64
                For i = 0 To 23
                    If s.T(i) = 0 Then
                        a = a Or (CType(1, Int64) << i)
                    ElseIf s.T(i) = 1 Then
                        a = a Or (CType(1, Int64) << (i + 24))
                    End If
                Next

                If s.SideToMove = 1 Then
                    a = BoardNegate(a)
                    id.negate()
                End If

                Dim sec = secs(id)

                Return sec.hash(a).Item2
            End SyncLock
        Catch ex As Exception
            If TypeOf ex Is KeyNotFoundException Then Throw
            MsgBox("Exception in Eval" & vbCrLf & ex.ToString, MsgBoxStyle.Critical)
            Environment.Exit(1)
        End Try
    End Function


    Private Function BoardNegate(ByVal a As Int64) As Int64
        Const mask24 As Int64 = (1 << 24) - 1
        Return ((a And mask24) << 24) Or ((a And (mask24 << 24)) >> 24)
    End Function
End Class
