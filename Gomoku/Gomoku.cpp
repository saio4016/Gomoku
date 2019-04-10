// Gomoku.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

/********************************************
Name      ：Gomoku.cpp
Function  ：Gomokunarabe
Author    ：saio4016
Date      ：2019/01/23(last update:2019/04/10)
Language  ：C
IDE/Editor：Visual Studio 2017
Time      ：1week
Notices   ：Use "Config.txt" and "Board.txt".
			This is an assignment at my University.
********************************************/

#include "pch.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#pragma warning(disable: 4996) //fopenのerror回避

//駒を数値で表す
#define NONE 0
#define BLACK 1
#define WHITE 2

void game_config(int *row, int *col, int *moku, int *mode, int *count, int *turn, int ***board, int ***sboard);
void new_config(int *row, int *col, int *moku, int *mode, int *count, int *turn);
void make_board(int row, int col, int ***board);
void make_sboard(int row, int col, int ***sboard);
int load_config(int *row, int *col, int *moku, int *mode, int *count, int *turn);
int load_board(int row, int col, int *count, int *turn, int **board, int **sboard);

void disp(int row, int col, int **board);
int input(int *x, int *y, int row, int col, int moku, int mode, int *count, int *turn, int **board, int **sboard);
void menu(int row, int col, int moku, int mode, int *count, int *turn, int **board, int **sboard);
void disp_config(int row, int col, int turn, int count, int moku);
void undo(int row, int col, int mode, int *count, int *turn, int **board, int **sboard);
void redo(int row, int col, int mode, int *count, int latest_count, int *turn, int **board, int **sboard);
void save(int row, int col, int moku, int mode, int count, int **sboard);
int judge_end(int x, int y, int row, int col, int moku, int mode, int count, int turn, int **board);
void disp_result(int turn, int count, int jud);
void game_end(int row, int col, int ***board, int ***sboard);
void change_turn(int *turn);

void CPU(int *x, int *y, int row, int col, int moku, int turn, int count, int **board);
int CPU_brain(int x, int y, int row, int col, int moku, int turn, int **board);

//右上から時計回りに8近傍
const int dx[] = { 1,1,1,0,-1,-1,-1,0 };
const int dy[] = { 1,0,-1,-1,-1,0,1,1 };

int main() {
	/*設定*/
	int row, col, moku, mode; //縦,横,並べる数,mode = (1:対人戦, 2:COM戦(先手), 3:COM戦(後手))
	int count, turn; //現在の手数,現在の手番
	int **board, **sboard; //盤面,置いた場所保存用([i][j]: i=(0:x,1:y)座標, j手目)
	game_config(&row, &col, &moku, &mode, &count, &turn, &board, &sboard); //初期設定
	printf("ボタンを押すと対局がスタートします"); _getch();
	printf("\n\n");

	/*対局*/
	while (1) {
		int x, y;//置く場所
		disp(row, col, board); printf("・%d手目 ", count);
		if (mode == 1 || (mode == 2 && (count % 2)) || mode == 3 && !(count % 2)) {
			//自分のターン
			if (input(&x, &y, row, col, moku, mode, &count, &turn, board, sboard)) continue;
		}
		else {
			//相手のターン
			CPU(&x, &y, row, col, moku, turn, count, board);
		}

		board[y][x] = turn; //駒を置く
		sboard[0][count - 1] = x, sboard[1][count - 1] = y;//置いた場所を記録する

		//終了判定(戻り値:1,勝利/敗北 2,反則負け 3,引き分け)
		int jud = judge_end(x, y, row, col, moku, mode, count, turn, board);
		if (jud) {
			disp(row, col, board);
			disp_result(turn, count, jud);
			break;
		}

		change_turn(&turn);
		count++; //手数を更新
	}

	game_end(row, col, &board, &sboard);
	return 0;
}

void game_config(int *row, int *col, int *moku, int *mode, int *count, int *turn, int ***board, int ***sboard) {
	while (1) {
		printf("=============(N目並べ)=============\n");
		printf(" 1: 新規ゲーム  2: ロード  3: 終了\n");
		printf("===================================\n");
		char str[256]; //入力用
		printf("--> "); fgets(str, 256, stdin);
		switch (strtol(str, NULL, 10)) {
		case 1:
			new_config(row, col, moku, mode, count, turn); //新規設定
			make_board(*row, *col,board); //盤面作成
			make_sboard(*row, *col,sboard); //保存用配列作成
			break;
		case 2:
			if (load_config(row, col, moku, mode, count, turn)) continue; //設定ロード
			make_board(*row, *col,board); //盤面作成
			make_sboard(*row, *col,sboard); //保存用配列作成
			if (load_board(*row, *col, count, turn, *board, *sboard)) continue; //盤面ロード
			break;
		case 3:
			exit(0);
		default:
			printf("Error\n");
			continue;
		}
		break;
	}
}

void new_config(int *row, int *col, int *moku, int *mode, int *count, int *turn) {
	printf(" ------------(設定)------------\n");
	char r[256], c[256], m[256]; //入力用
	while (1) {
		printf("・高さ(3~99) --> "); fgets(r, 256, stdin);
		printf("・幅(3~99)   --> "); fgets(c, 256, stdin);
		*row = strtol(r, NULL, 10), *col = strtol(c, NULL, 10); //数値変換
		if (!(3 <= *row && *row <= 99 && 3 <= *col && *col <= 99)) {
			//不正入力
			printf("Error\n");
			continue;
		}
		printf("・つなげる数(3~%d) --> ", (*row > *col ? *row : *col));
		fgets(m, 256, stdin); *moku = strtol(m, NULL, 10); //数値変換
		if (*moku < 3 || (*row < *moku && *col < *moku)) {
			//不正入力
			printf("Error\n");
			continue;
		}
		break;
	}
	while (1) {
		printf("・対戦モード\n");
		printf(" 1: 対人戦\n");
		printf(" 2: COM戦(先手)\n");
		printf(" 3: COM戦(後手)\n");
		printf(" --> ");
		fgets(m, 256, stdin); *mode = strtol(m, NULL, 10); //入力&数値変換
		if (!(1 <= *mode && *mode <= 3)) {
			printf("Error\n");
			continue;
		}
		break;
	}
	*count = 1; //現在の手数を1に設定
	*turn = BLACK;
	printf("-----------------------------\n");
}

void make_board(int row, int col, int ***board) {
	*board = (int**)malloc(sizeof(int*)*row);
	for (int i = 0; i < row; i++) (*board)[i] = (int*)malloc(sizeof(int)*col);
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			(*board)[i][j] = NONE;
		}
	}
}

void make_sboard(int row, int col, int ***sboard) {
	*sboard = (int**)malloc(sizeof(int*)*(2));
	for (int i = 0; i < 2; i++) (*sboard)[i] = (int*)malloc(sizeof(int)*(row*col));
	for (int i = 0; i < row*col; i++) {
		(*sboard)[0][i] = (*sboard)[1][i] = -1;
	}
}

int load_config(int *row, int *col, int *moku, int *mode, int *count, int *turn) {
	FILE *fp;
	//ファイル読み込み失敗
	if ((fp = fopen("Config.txt", "r")) == NULL) {
		printf("File open error\n");
		exit(1);
	}
	//データがない場合
	if (fgetc(fp) == EOF) {
		printf("No Data\n");
		fclose(fp);
		return 1;
	}
	else rewind(fp); //ファイルポインタを元に戻す
   /*データが壊れている場合は想定しない*/
	fscanf(fp, "%d %d %d %d %d", row, col, moku, mode, count); //保存データを読み込み
	*turn = BLACK;
	fclose(fp);
	return 0;
}

int load_board(int row, int col, int *count, int *turn, int **board, int **sboard) {
	FILE *fp;
	//ファイル読み込み失敗
	if ((fp = fopen("Board.txt", "r")) == NULL) {
		printf("File open error\n");
		exit(1);
	}
	//データがない場合
	if (fgetc(fp) == EOF) {
		printf("No Data\n");
		fclose(fp);
		return 1;
	}
	else rewind(fp); //ファイルポインタを元に戻す
   //config.txtに対応しない場合は想定しない
	for (int i = 0; i < *count - 1; i++) {
		fscanf(fp, "%d", &sboard[0][i]);
	}
	for (int i = 0; i < *count - 1; i++) {
		fscanf(fp, "%d", &sboard[1][i]);
	}
	//実際に局面を進める
	for (int i = 0; i < *count - 1; i++) {
		board[sboard[1][i]][sboard[0][i]] = *turn;
		change_turn(turn);
	}
	fclose(fp);
	return 0;
}

void disp(int row, int col, int **board) {
	printf("   ");
	for (int i = 0; i < col; i++) printf("%02d ", i + 1); //横軸
	printf("\n  ");
	for (int i = 0; i < col; i++) printf("+--");
	printf("+\n");
	for (int i = 0; i < row; i++) {
		printf("%02d", i + 1); //縦軸表示
		for (int j = 0; j < col; j++) {
			switch (board[i][j]) {
			case NONE:
				printf("|  ");
				break;
			case BLACK:
				printf("|○");
				break;
			case WHITE:
				printf("|●");
				break;
			default:
				printf("|×");
				break;
			}
		}
		printf("|\n  ");
		for (int i = 0; i < col; i++) printf("+--");
		printf("+\n");
	}
}

int input(int *x, int *y, int row, int col, int moku, int mode, int *count, int *turn, int **board, int **sboard) {
	char xx[256], yy[256]; //入力用
	printf("%s", *turn == BLACK ? "先手○" : "後手●");
	if (mode == 2 || mode == 3) printf("(あなたの番)");
	printf(":(0,0)でメニューを開く\n");
	printf(" x座標--> "); fgets(xx, 256, stdin);
	printf(" y座標--> "); fgets(yy, 256, stdin);
	if (strlen(xx) == 2 && strlen(yy) == 2 && xx[0] == '0' && yy[0] == '0') {
		menu(row, col, moku, mode, count, turn, board, sboard);
		return 1; //->再度盤面出力
	}
	*x = strtol(xx, NULL, 10), *y = strtol(yy, NULL, 10); //数値に変換
	(*x)--, (*y)--; //board[y][x]で表現できるように1を引く
	if (!(0 <= *x && *x < col && 0 <= *y && *y < row && board[*y][*x] == NONE)) {
		//入力が不正な場合
		printf("入力が不正です\n");
		return 1;
	}
	return 0;
}

void menu(int row, int col, int moku, int mode, int *count, int *turn, int **board, int **sboard) {
	char str[256]; //入力用
	int latest_count = *count; //最新の手(進められる限界)
	while (1) {
		printf("-----------------menu-----------------\n");
		printf(" 1:設定確認  2:一手戻す  3:一手進める\n");
		printf(" 4:セーブ  5:対局に戻る  6:対局終了\n");
		printf("--------------------------------------\n");
		printf("--> "); fgets(str, 256, stdin);
		switch (strtol(str, NULL, 10)) {
		case 1: //設定確認
			disp_config(row, col, moku, mode, *count);
			continue;
		case 2:
			undo(row, col, mode, count, turn, board, sboard);
			continue;
		case 3:
			redo(row, col, mode, count, latest_count, turn, board, sboard);
			continue;
		case 4: //セーブ
			save(row, col, moku, mode, *count, sboard);
			printf("セーブしました\n");
			continue;
		case 5: //戻る
			break;
		case 6: //やめる
			exit(0);
		default:
			printf("入力が不正です\n");
			continue;
		}
		break;
	}
}

void disp_config(int row, int col, int moku, int mode, int count) {
	printf("・対局設定\n");
	printf("縦の長さ: %d\n", row);
	printf("横の長さ: %d\n", col);
	printf("つなげる数: %d\n", moku);
	printf("対戦モード: ");
	if (mode == 1) printf("対人戦\n");
	else printf("CPU戦(%s)\n", (mode == 2 ? "先手" : "後手"));
	printf("現在の手数: %d\n\n", count);
	printf("・ルール\n");
	printf("%d連: ちょうど%d個の連を作った方が勝ち\n", moku, moku);
	printf("長連: 先手が%d個以上の連を作ったら反則負け\n\n", moku + 1);
}

void undo(int row, int col, int mode, int *count, int *turn, int **board, int **sboard) {
	if (*count == 1) {
		printf("Error\n");
	}
	else {
		for (int i = 0; i < (mode == 1 ? 1 : 2); i++) {
			int x = sboard[0][*count - 2], y = sboard[1][*count - 2];
			board[y][x] = NONE;
			change_turn(turn);
			(*count)--;
		}
		disp(row, col, board);
		printf("%d手戻しました\n", (mode == 1 ? 1 : 2));
	}
}

void redo(int row, int col, int mode, int *count, int latest_count, int *turn, int **board, int **sboard) {
	if (*count == latest_count) {
		printf("Error\n");
	}
	else {
		for (int i = 0; i < (mode == 1 ? 1 : 2); i++) {
			int x = sboard[0][*count - 1], y = sboard[1][*count - 1];
			board[y][x] = *turn;
			change_turn(turn);
			(*count)++;
		}
		disp(row, col, board);
		printf("%d手進めました\n", (mode == 1 ? 1 : 2));
	}
}

void save(int row, int col, int moku, int mode, int count, int **sboard) {
	FILE *fp;
	//ファイル読み込み失敗
	if ((fp = fopen("Config.txt", "w")) == NULL) {
		printf("File open error\n");
		exit(1);
	}
	fprintf(fp, "%d %d %d %d %d\n", row, col, moku, mode, count); //データの保存
	fclose(fp);
	if ((fp = fopen("Board.txt", "w")) == NULL) {
		printf("File open error\n");
		exit(1);
	}
	//セーブできるのは(row*col-1)までなので配列を超すことはない
	for (int i = 0; i < count - 1; i++) {
		fprintf(fp, "%d ", sboard[0][i]); //データを保存 
	}
	fprintf(fp, "\n");
	for (int i = 0; i < count - 1; i++) {
		fprintf(fp, "%d ", sboard[1][i]); //データを保存 
	}
	fclose(fp);
}

int judge_end(int x, int y, int row, int col, int moku, int mode, int count, int turn, int **board) {
	int win = 0, lose = 0;
	//置いたところから探索
	for (int k = 0; k < 4; k++) {
		int nx = x, ny = y;
		int cnt = 1; //つながってる個数
		//k が進むと 8 近傍の右上から下まで時計回りに移動
		//(nx,ny) から (nx+dx[k],ny+dy[k]) に進みそこが盤面外または色が変われば break
		while (1) {
			nx += dx[k], ny += dy[k]; //進む
			if (nx < 0 || col <= nx || ny < 0 || row <= ny) break;
			if (board[ny][nx] != turn) break;
			cnt++;
		}
		nx = x, ny = y; //x,y リセット
		//k が進むと 8 近傍の左下から上まで時計回りに移動 (上の探索と対角線上逆向き)
		while (1) {
			nx += dx[k + 4], ny += dy[k + 4];
			if (nx < 0 || col <= nx || ny < 0 || row <= ny) break;
			if (board[ny][nx] != turn) break;
			cnt++;
		}
		if (turn == BLACK && cnt > moku) lose = 1; //反則判定
		else if (cnt >= moku) win = 1; //勝利判定
	}
	if (lose) return 2;
	if (win) return 1;
	if (row*col == count) return 3; //引き分け
	return 0;
}

void disp_result(int turn, int count, int jud) {
	switch (jud) {
	case 1: //勝利
		printf("まで%d手 %sの勝ち\n", count, (turn == BLACK ? "先手" : "後手"));
		break;
	case 2: //反則
		printf("まで%d手 黒の反則負け\n", count);
		break;
	case 3: //引き分け
		printf("引き分け\n");
		break;
	}
}

void game_end(int row, int col, int ***board, int ***sboard) {
	for (int i = 0; i < row; i++) {
		free((*board)[i]);
	}
	free(*board);
	for (int i = 0; i < 2; i++) {
		free((*sboard)[i]);
	}
	free(*sboard);
}

void change_turn(int *turn) {
	*turn = (*turn == BLACK ? WHITE : BLACK);
}

void CPU(int *x, int *y, int row, int col, int moku, int turn, int count, int **board) {
	srand(time(NULL));
	if (count == 1) {
		//初手は真ん中に置く
		*x = col / 2;
		*y = row / 2;
	}
	else {
		int max = -1, tmp = 0;
		//すべての場所から探索
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				if (board[i][j] == NONE) {
					tmp = CPU_brain(j, i, row, col, moku, turn, board);
					//評価がmaxより大きい/評価がmaxと同じなら3分の1の確率
					if (max < tmp || (max == tmp && rand() % 3 == 0)) {
						*x = j, *y = i;
						max = tmp;
					}
				}
			}
		}
	}
	printf("%s(CPUの番):\n", (turn == BLACK ? "先手○" : "後手●"));
	printf(" x座標--> %d\n", *x + 1);
	printf(" y座標--> %d\n", *y + 1);
}

int CPU_brain(int x, int y, int row, int col, int moku, int turn, int **board) {
	int sum = 0; //戻り値
	for (int k = 0; k < 4; k++) {
		int sub = 0, cnt1 = 0, cnt2 = 0; //subは保存用,cntは各方向の並んでる数
		//8近傍の右上から下まで時計回りに探索
		int nx1 = x + dx[k], ny1 = y + dy[k];
		if (0 <= nx1 && nx1 < col && 0 <= ny1 && ny1 < row && board[ny1][nx1] != NONE) {
			cnt1++; //探索先に駒がある
			while (1) {
				nx1 += dx[k], ny1 += dy[k];
				//一つ進んだところの駒が盤面外/同じ色ではないときbreak
				if (nx1 < 0 || col <= nx1 || ny1 < 0 || row <= ny1) break;
				if (board[ny1][nx1] != board[ny1 - dy[k]][nx1 - dx[k]]) break;
				cnt1++; //さらに進める
			}
		}
		//8近傍の左下から上まで時計回りに探索(上の探索の対角線上逆向き)
		int nx2 = x + dx[k + 4], ny2 = y + dy[k + 4];
		if (0 <= nx2 && nx2 < col && 0 <= ny2 && ny2 < row && board[ny2][nx2] != NONE) {
			cnt2++; //以後上と同じ
			while (1) {
				nx2 += dx[k + 4], ny2 += dy[k + 4];
				if (nx2 < 0 || col <= nx2 || ny2 < 0 || row <= ny2) break;
				if (board[ny2 - dy[k + 4]][nx2 - dx[k + 4]] != board[ny2][nx2]) break;
				cnt2++;
			}
		}
		nx1 = x + dx[k], ny1 = y + dy[k], nx2 = x + dx[k + 4], ny2 = y + dy[k + 4];
		//調べた2つの方向の色が同じ((x,y)に置くと(cnt1+1+cnt2)個つながる)
		if (0 <= nx1 && nx1 < col && 0 <= ny1 && ny1 < row &&
			0 <= nx2 && nx2 < col && 0 <= ny2 && ny2 < row &&
			board[ny1][nx1] == board[ny2][nx2]) sub = cnt1 + cnt2;
		//調べた二つの方向の色が違う(max(cnt1,cnt2)をとる)
		else sub = (cnt1 > cnt2 ? cnt1 : cnt2);
		if (sub >= moku - 2) sub *= 20; //必死の可能性あり
		if (sub >= moku - 1) sub *= 200; //王手の可能性あり
		sum += sub * sub; //4方向に操作を行う
	}
	return sum;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
